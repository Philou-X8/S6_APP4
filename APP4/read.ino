hw_timer_t *read_timer = NULL;

//read_timer = timerBegin(1000000);

volatile SemaphoreHandle_t LockState_0_1;
volatile SemaphoreHandle_t LockState_5_6;
volatile SemaphoreHandle_t LockStartFlag;

volatile int callibrate_counter = 0;
volatile unsigned long calib_start = 0;
volatile unsigned long calib_end = 0;
//volatile int read_priode = 0;
volatile int read_max_delay = 0;

volatile int reader_state = 0;

volatile unsigned long last_bit_time = 0;
volatile unsigned int read_buffer = 0;
volatile int read_buffer_size = 0;
//volatile int total_bit_counter;

//-----------------------------------------
//--------------- Tasks -------------------
//-----------------------------------------


void TaskReceive(void *pvParameters) {
  Serial.println("TaskReceive tread created");

  read_timer = timerBegin(4000000);
  //timerStart(read_timer);

  LockState_0_1 = xSemaphoreCreateBinary();
  LockState_5_6 = xSemaphoreCreateBinary();
  LockStartFlag = xSemaphoreCreateBinary();

  //int reader_state = 0;

  unsigned long read_timer_start = 0;
  unsigned long read_timer_end = 0;

  char message_buff[88] = { 0 };
  int message_buff_index = 0;
  

  attachInterrupt(digitalPinToInterrupt(READ_PIN), InterruptSync, RISING);

  Serial.println("TaskReceive tread, ready to read");

  //start_time = micros();
  while (1) {
    delayMicroseconds(3);
    //delay(1);

    if (reader_state == 0) { // sync clock
      if (xSemaphoreTake(LockState_0_1, 0) == pdTRUE){
        //detachInterrupt(digitalPinToInterrupt(READ_PIN));
        read_timer_start = micros();
        reader_state = 1;
      }
      read_timer_start = micros();

    } 
    else if (reader_state == 1) { // start searching for Start flag
      // detach interrupt
      detachInterrupt(digitalPinToInterrupt(READ_PIN));
      attachInterrupt(digitalPinToInterrupt(READ_PIN), InterruptRead, CHANGE);

      read_buffer = 0;
      read_buffer_size = 0;
      //message_buff = ""; // fix resetting the read buffer
      message_buff_index = 0;

      reader_state = 2;
    } 
    else if (reader_state == 2) { // wait for end of start flag
      //int temp_buffer = read_buffer;
      //int temp_buffer_size = read_buffer_size;

      if (xSemaphoreTake(LockStartFlag, 0) == pdTRUE){
        reader_state = 3;
      }

      /*
      for (int i = (temp_buffer_size - 3); i >= 0; i--) {

        if (((temp_buffer >> i) & 0x7) == 0x6) {  // search for the first instance of '110' which mark the end of the Start flag
          read_buffer = temp_buffer % (0x1 << i);
          read_buffer_size = i;
          message_buff_index = 0;
          reader_state = 3;  // change state
          //Serial.println("HEADER FOUND");
          break;
        }
      }
      */

      if (read_buffer_size >= 16) {  // start flag not found
        Serial.println("ERROR: START FLAG NOT FOUND IN MESSAGE");
        // Try to not fall in a death loop, idk

        detachInterrupt(digitalPinToInterrupt(READ_PIN));
        attachInterrupt(digitalPinToInterrupt(READ_PIN), InterruptIdle, CHANGE);
        reader_state = 4; 
      }
    } 
    else if (reader_state == 3) { // read transmission and fill the byte buffer
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

      if (message_buff_index >= 85){ // we should have all the message
        reader_state = 4;
      } 
    }
    else if (reader_state == 4) { // parse the message and check CRC
      // stop reading the pin
      detachInterrupt(digitalPinToInterrupt(READ_PIN));

      read_timer_end = micros();

      if( CheckCRC(message_buff) ){
        Serial.println("CRC MISSMATCH");
      }


      Serial.print("Transmittion complete in ");
      Serial.print(read_timer_end - read_timer_start);
      Serial.println(" us, here's the raw message:");
      //Serial.println(String(message_buff).substring(4, 83).c_str()); // doesn't fucking work for some reason
      //Serial.println("-------------");
      for(int i = 3; i < 83; i++){
        //Serial.print(int(message_buff[i]));
        //Serial.print(", ");
        Serial.print(message_buff[i]);
      }
      Serial.println("\n-------------");
      //Serial.println(String(true_message).substring(4, 8).c_str());
      
      callibrate_counter = 0;
      read_max_delay = 0;

      read_buffer = 0;
      read_buffer_size = 0;
      //total_bit_counter = 0;
      
      //delay(1000);

      // go to back to standby mode
      attachInterrupt(digitalPinToInterrupt(READ_PIN), InterruptIdle, CHANGE);
      reader_state = 5; 
    }
    else if (reader_state == 5) { // IDLE
      if (xSemaphoreTake(LockState_5_6, 0) == pdTRUE){
        //detachInterrupt(digitalPinToInterrupt(READ_PIN));
        reader_state = 6;
      }
    }
    else if (reader_state == 6) { // reset system and return to sync state
      callibrate_counter = 0;
      detachInterrupt(digitalPinToInterrupt(READ_PIN));
      attachInterrupt(digitalPinToInterrupt(READ_PIN), InterruptSync, RISING);
      
      read_max_delay = 0;

      read_buffer = 0;
      read_buffer_size = 0;
      //total_bit_counter = 0;
      //Serial.println("READER STATE: 6 -> 0");
      
      reader_state = 0;
    }


  }
}


//-----------------------------------------
//-------------- Interrupt ----------------
//-----------------------------------------

void InterruptIdle(){
  //xSemaphoreGiveFromISR(LockState_5_6, NULL);
  reader_state = 6;
}

void InterruptSync() {  // must call on rising edge

  if (callibrate_counter == 1) {
    //calib_start = timerReadMicros(read_timer);
    calib_start = micros();
  } 
  
  else if (callibrate_counter == 3) {
    //calib_end = timerReadMicros(read_timer);
    calib_end = micros();
    //Serial.println(calib_end - calib_start);
  } 
  else if (callibrate_counter == 4) {
    read_max_delay = ((calib_end - calib_start) >> 3) + ((calib_end - calib_start) >> 4); // + ((calib_end - calib_start) >> 6); // 1/8 + 1/16 = 3/16
    //xSemaphoreGiveFromISR(LockState_0_1, NULL);
    //Serial.println(read_max_delay);
    reader_state = 1;
  }

  callibrate_counter += 1;
}

//volatile int pin_state = 0;
void InterruptRead() {

  if ((micros() - last_bit_time) > read_max_delay){
  //if ((timerReadMicros(read_timer) - last_bit_time) > read_max_delay){
    //last_bit_time = micros();

    // shift the read_buffer once to make room, then add the new bit to the left
    read_buffer = (read_buffer << 1) | ((GPIO.in >> READ_PIN) & 0x1);
    read_buffer_size++; // size of the read buffer

    if(reader_state == 2){
      if( (read_buffer & 0x7) == 0x6){
        //pin_state = 1;
        read_buffer = 0;
        read_buffer_size = 0;
        xSemaphoreGiveFromISR(LockStartFlag, NULL);
      }
    }

    last_bit_time = micros();

    //((GPIO.in >> READ_PIN) & 0x1) ? Serial.println("detect 1") : Serial.println("detect 0");
  }
}

//-----------------------------------------
//------------- Functions -----------------
//-----------------------------------------

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
