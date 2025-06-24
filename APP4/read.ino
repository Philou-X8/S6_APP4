

volatile int callibrate_counter = 0;
volatile unsigned long calib_start = 0;
volatile unsigned long calib_end = 0;
volatile int read_priode = 0;
volatile int read_max_delay = 0;
volatile int reader_state = 0;


volatile unsigned long last_bit_time = 0;
volatile unsigned int read_buffer = 0;
volatile int read_buffer_size;
volatile int total_bit_counter;

//-----------------------------------------
//--------------- Tasks -------------------
//-----------------------------------------


void TaskReceive(void *pvParameters) {
  Serial.println("TaskReceive tread");

  unsigned long read_timer_start = 0;
  unsigned long read_timer_end = 0;

  char message_buff[88] = { 0 };
  int message_buff_index = 0;
  

  attachInterrupt(digitalPinToInterrupt(READ_PIN), InterruptSync, RISING);

  Serial.println("TaskReceive tread, ready to read");

  //start_time = micros();
  while (1) {
    delayMicroseconds(10);
    //delay(1);

    if (reader_state == 0) {
      read_timer_start = micros();

    } else if (reader_state == 1) {
      // detach interrupt
      detachInterrupt(digitalPinToInterrupt(READ_PIN));

      read_buffer = 0;
      read_buffer_size = 0;
      //message_buff = ""; // fix resetting the read buffer
      message_buff_index = 0;

      attachInterrupt(digitalPinToInterrupt(READ_PIN), InterruptRead, CHANGE);
      reader_state = 2;
    } 
    else if (reader_state == 2) {
      for (int i = (read_buffer_size - 3); i >= 0; i--) {

        if (((read_buffer >> i) & 0x7) == 0x6) {  // search for the first instance of '110' which mark the end of the Start flag
          read_buffer = read_buffer % (0x1 << i);
          read_buffer_size = i;
          message_buff_index = 0;
          reader_state = 3;  // change state
          //Serial.println("HEADER FOUND");
          break;
        }
      }

      if (read_buffer_size >= 16) {  // start flag not found
        Serial.println("ERROR: START FLAG NOT FOUND IN MESSAGE");
        // Try to not fall in a death loop, idk

        detachInterrupt(digitalPinToInterrupt(READ_PIN));
        attachInterrupt(digitalPinToInterrupt(READ_PIN), InterruptIdle, CHANGE);
        reader_state = 4; 
      }
    } 
    else if (reader_state == 3) {  // just read for now, we'll parse later
      if (read_buffer_size >= 8) {
        int temp = read_buffer >> (read_buffer_size - 8);
        message_buff[message_buff_index] = (char)temp;

        //Serial.print("recieved character ");
        //Serial.print(message_buff_index);
        //Serial.print(" : ");
        //Serial.println(message_buff[message_buff_index]);

        //read_in_buffer = read_in_buffer ^ (temp << (read_buffer_size - 8));
        read_buffer = read_buffer % (0x1 << (read_buffer_size - 8));  // keep only the last  bits
        read_buffer_size -= 8;

        message_buff_index++;
      }

      if (message_buff_index > 85){ // we should have all the message
        reader_state = 4;
      } 
    }
    else if (reader_state == 4){ // parse the message and shit
      // stop reading the pin
      detachInterrupt(digitalPinToInterrupt(READ_PIN));

      if( CheckCRC(message_buff) ){
        Serial.println("CRC MISSMATCH");
      }

      read_timer_end = micros();

      Serial.print("Transmittion complete in ");
      Serial.print(read_timer_end - read_timer_start);
      Serial.println(" us, here's the raw message:");
      //Serial.println(String(message_buff).substring(4, 83).c_str()); // doesn't fucking work for some reason
      Serial.println("-------------");
      for(int i = 3; i < 83; i++){
        Serial.print(message_buff[i]);
      }
      Serial.println("\n-------------");
      //Serial.println(String(true_message).substring(4, 8).c_str());
      
      callibrate_counter = 0;
      read_max_delay = 0;

      read_buffer = 0;
      read_buffer_size = 0;
      total_bit_counter = 0;
      
      //delay(1000);

      // go to back to standby mode
      attachInterrupt(digitalPinToInterrupt(READ_PIN), InterruptIdle, CHANGE);
      reader_state = 5; 
    }
    else if (reader_state == 5){
      // state change done by the interrupt
    }
    else if (reader_state == 6){
      detachInterrupt(digitalPinToInterrupt(READ_PIN));
      
      callibrate_counter = 0;
      read_max_delay = 0;

      read_buffer = 0;
      read_buffer_size = 0;
      total_bit_counter = 0;
      Serial.println("READER STATE: 6 -> 0");
      
      attachInterrupt(digitalPinToInterrupt(READ_PIN), InterruptSync, RISING);
      reader_state = 0;
    }


  }
}


//-----------------------------------------
//-------------- Interrupt ----------------
//-----------------------------------------

void InterruptIdle(){
  reader_state = 6;
}
void InterruptSync() {  // must call on rising edge

  if (callibrate_counter == 1) {
    calib_start = micros();
  } 
  else if (callibrate_counter == 3) {
    calib_end = micros();
    read_priode = (calib_end - calib_start);
    read_max_delay = (read_priode >> 3) + (read_priode >> 4); // 1/8 + 1/16 = 3/16
    //Serial.print("read_max_delay = ");
    //Serial.println(read_max_delay);
  } 
  else if (callibrate_counter == 4) {
    if(reader_state == 0){

      //Serial.println("changing from state 0 to state 1");
      reader_state = 1;
    }
  }

  //Serial.println(callibrate_counter);
  callibrate_counter += 1;
}

void InterruptRead() {
  if ((micros() - last_bit_time) > read_max_delay){
    // shift the read_buffer once to make room, then add the new bit to the left
    read_buffer = (read_buffer << 1) | ((GPIO.in >> READ_PIN) & 0x1);
    read_buffer_size++; // size of the read buffer
    total_bit_counter++;
    last_bit_time = micros();

    //((GPIO.in >> READ_PIN) & 0x1) ? Serial.println("detect 1") : Serial.println("detect 0");
  }
}

//-----------------------------------------
//------------- Functions -----------------
//-----------------------------------------

// TODO: ADJUST INDEX FOR MESSAGE BUFFER <------------------------------------- TODO
static inline int CheckCRC(const char *a_payload) {
  const int start_i = 3;  // data payload start at byte 5
  const int end_i = 83;   // data payload end at byte 5
  unsigned short crc = 0;

  for (int i = start_i; i < (end_i + 2); i += 1) {
    crc = crc ^ (a_payload[i] << 8);

    for (int b = 0; b < 8; b++) {

      if (crc & 0x8000) {
        crc = (crc << 1) ^ CRC_MASK;
      } else {
        crc = (crc << 1);
      }
    }
  }
  return crc;  // (crc > 0) if there's an error
}
