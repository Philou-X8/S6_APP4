
#define CRC_MASK 0x1021  // CRC-16-CCITT

#define TANSMIT_CORE 0
#define RECEIVE_CORE 1

#define SEND_PIN 25
#define READ_PIN 26

#define WRITE_SPEED 11 // time in us. (micro sec)


volatile int write_bit_index = 0;

volatile SemaphoreHandle_t writerSemaphore;
portMUX_TYPE sendBitMUX = portMUX_INITIALIZER_UNLOCKED;


//-----------------------------------------
//--------------- Tasks -------------------
//-----------------------------------------

void TaskTransmitV2(void *pvParameters){

  char *a_payload = (char *)pvParameters;

  digitalWrite(SEND_PIN, 0);

  writerSemaphore = xSemaphoreCreateBinary();
  // Set timer frequency to 1Mhz
  timer = timerBegin(1000000);  //

  timerAttachInterrupt(timer, &SendBitV2);

  // Set alarm to call onTimer function every second (value in microseconds).
  // Repeat the alarm (third parameter) with unlimited count = 0 (fourth parameter).
  timerAlarm(timer, WRITE_SPEED, false, 0);

  char text_payload[80] = "This is a testing message. It doesnt mean anything, it just need to use 80 char";
  
  int message_version = 0;
  unsigned long send_timer_start = 0;
  unsigned long send_timer_end = 0;

  delay(1000);

  //------------------

  int byte_progress = 0;
  unsigned short crc = 0;

  send_timer_start = micros();

  while(1){
    delayMicroseconds(WRITE_SPEED);

    if(byte_progress == 0){
      send_timer_start = micros();
      timerAlarm(timer, WRITE_SPEED, true, 0);
    }
      
    if(byte_progress < 80){
      char current_byte = text_payload[byte_progress];

      // compute CRC
      crc = crc ^ (current_byte << 8);
      for (int b = 0; b < 8; b++) {

        if (crc & 0x8000) {
          crc = (crc << 1) ^ 0x1021;
        } else {
          crc = (crc << 1);
        }
      }

      char manch_l = ManchesterLeft(current_byte);
      char manch_r = ManchesterRight(current_byte);

      portENTER_CRITICAL(&sendBitMUX);
      a_payload[(byte_progress + 5) * 2] = manch_l;
      a_payload[(byte_progress + 5) * 2 + 1] = manch_r;
      portEXIT_CRITICAL(&sendBitMUX);

    }
    else if (byte_progress == 80){
      char manch_l = ManchesterLeft(char(crc >> 8));
      char manch_r = ManchesterRight(char(crc >> 8));

      portENTER_CRITICAL(&sendBitMUX);
      a_payload[170] = manch_l;
      a_payload[171] = manch_r;
      portEXIT_CRITICAL(&sendBitMUX);
    }
    else if (byte_progress == 81){
      char manch_l = ManchesterLeft(char(crc & 0xFFFF));
      char manch_r = ManchesterRight(char(crc & 0xFFFF));

      portENTER_CRITICAL(&sendBitMUX);
      a_payload[172] = manch_l;
      a_payload[173] = manch_r;
      portEXIT_CRITICAL(&sendBitMUX);

      //Serial.print("CRC compute: ");
      //Serial.println(micros() - send_timer_start);
    }

    byte_progress++;

    if (xSemaphoreTake(writerSemaphore, 0) == pdTRUE){
      send_timer_end = micros();
      timerAlarm(timer, WRITE_SPEED, false, 0);

      byte_progress = 0;
      crc = 0;
      
      //Serial.print("Message done sending in ");
      //Serial.print(send_timer_end - send_timer_start);
      //Serial.println(" us.");

      digitalWrite(SEND_PIN, 0);
      
      message_version = (message_version + 1) % 10;
      text_payload[73] = char(48 + message_version);
      delay(3000);
    }

  } // thread's While()
  
}

//-----------------------------------------
//-------------- Interrupt ----------------
//-----------------------------------------


void ARDUINO_ISR_ATTR SendBitV2() {
  // mutex start
  char data_byte = 0;
  // mutex end

  portENTER_CRITICAL(&sendBitMUX);
  data_byte = g_payload[(write_bit_index >> 3)];
  portEXIT_CRITICAL(&sendBitMUX);

  //data_byte = 153; // TEMP ------------------------------------ FOR TESTING

  if( (data_byte >> (7 - (write_bit_index & 0x7))) & 0x1 ){
    GPIO.out_w1ts = (0x1 << SEND_PIN);
    //Serial.println("writing bit 1");
  }
  else{
    GPIO.out_w1tc = (0x1 << SEND_PIN);
    //Serial.println("writing bit 0");
  }

  write_bit_index++;
  if(write_bit_index > 1408){
    write_bit_index = 0;
    xSemaphoreGiveFromISR(writerSemaphore, NULL);
  }
}


void ARDUINO_ISR_ATTR SendBit() {
  // mutex start
  int byte_index = (write_bit_index >> 3);  // last 3 bits are used to count the bit position
  int bit_index = (write_bit_index & 0x7);  // 0b0111
  write_bit_index++;
  // mutex end

  if((g_payload[byte_index] >> (7-bit_index)) & 0x1){
    GPIO.out_w1ts = (0x1 << SEND_PIN);
    //Serial.println("writing bit 1");
  }
  else{
    GPIO.out_w1tc = (0x1 << SEND_PIN);
    //Serial.println("writing bit 0");
  }

  if(write_bit_index > 1408){
    write_bit_index = 0;
    xSemaphoreGiveFromISR(writerSemaphore, NULL);
  }
}



//-----------------------------------------
//------------- Functions -----------------
//-----------------------------------------


static inline void ComputeCRC(char *payload_w) {
  const int start_i = 10;  // data payload start at byte 5*2=10
  const int end_i = 170;   // data payload end at byte 85*2=170
  unsigned short crc = 0;

  for (int i = start_i; i < end_i; i += 2) {
    crc = crc ^ (payload_w[i] << 8);

    for (int b = 0; b < 8; b++) {

      if (crc & 0x8000) {
        crc = (crc << 1) ^ CRC_MASK;
      } else {
        crc = (crc << 1);
      }
    }
  }
  payload_w[end_i + 0] = crc >> 8;
  payload_w[end_i + 2] = crc & 0xFFFF;
}


static inline void MakeManchester(char *payload_w) {
  const int start_i = 10;     // data payload start at byte 5*2=10
  const int end_i = 168;      // data payload end at byte (85-1)*2=168
  const int crc_end_i = 172;  // data payload end at byte 85*2=170

  char read_buffer = 0;
  for (int i = start_i; i <= crc_end_i; i += 2) {
    read_buffer = payload_w[i];
    //read_buffer = 0x5a;
    //std::cout << "manch buff: " << PrintByte(read_buffer) << std::endl;

    payload_w[i] = ((read_buffer & 0x80 ^ 0x80) >> 0)
                   | ((read_buffer & 0x80) >> 1)
                   | ((read_buffer & 0x40 ^ 0x40) >> 1)
                   | ((read_buffer & 0x40) >> 2)

                   | ((read_buffer & 0x20 ^ 0x20) >> 2)
                   | ((read_buffer & 0x20) >> 3)
                   | ((read_buffer & 0x10 ^ 0x10) >> 3)
                   | ((read_buffer & 0x10) >> 4);

    payload_w[i + 1] = ((read_buffer & 0x08 ^ 0x08) << 4)
                       | ((read_buffer & 0x08) << 3)
                       | ((read_buffer & 0x04 ^ 0x04) << 3)
                       | ((read_buffer & 0x04) << 2)

                       | ((read_buffer & 0x02 ^ 0x02) << 2)
                       | ((read_buffer & 0x02) << 1)
                       | ((read_buffer & 0x01 ^ 0x01) << 1)
                       | ((read_buffer & 0x01) << 0);

    //std::cout << PrintByte(payload_w[i]) << " " << PrintByte(payload_w[i + 1]) << std::endl;
  }
}


static inline void MakeManchester(char *payload_w, int index) {

  char read_buffer = payload_w[index];

  //std::cout << "manch buff: " << PrintByte(read_buffer) << std::endl;

  payload_w[index] = ((read_buffer & 0x80 ^ 0x80) >> 0)
                     | ((read_buffer & 0x80) >> 1)
                     | ((read_buffer & 0x40 ^ 0x40) >> 1)
                     | ((read_buffer & 0x40) >> 2)

                     | ((read_buffer & 0x20 ^ 0x20) >> 2)
                     | ((read_buffer & 0x20) >> 3)
                     | ((read_buffer & 0x10 ^ 0x10) >> 3)
                     | ((read_buffer & 0x10) >> 4);

  payload_w[index + 1] = ((read_buffer & 0x08 ^ 0x08) << 4)
                         | ((read_buffer & 0x08) << 3)
                         | ((read_buffer & 0x04 ^ 0x04) << 3)
                         | ((read_buffer & 0x04) << 2)

                         | ((read_buffer & 0x02 ^ 0x02) << 2)
                         | ((read_buffer & 0x02) << 1)
                         | ((read_buffer & 0x01 ^ 0x01) << 1)
                         | ((read_buffer & 0x01) << 0);

  //std::cout << PrintByte(payload_w[index]) << " " << PrintByte(payload_w[index + 1]) << std::endl;
}


static inline char ManchesterLeft(char raw){
  return ((raw & 0x80 ^ 0x80) >> 0)
                     | ((raw & 0x80) >> 1)
                     | ((raw & 0x40 ^ 0x40) >> 1)
                     | ((raw & 0x40) >> 2)

                     | ((raw & 0x20 ^ 0x20) >> 2)
                     | ((raw & 0x20) >> 3)
                     | ((raw & 0x10 ^ 0x10) >> 3)
                     | ((raw & 0x10) >> 4);
}

static inline char ManchesterRight(char raw){
  return ((raw & 0x08 ^ 0x08) << 4)
                         | ((raw & 0x08) << 3)
                         | ((raw & 0x04 ^ 0x04) << 3)
                         | ((raw & 0x04) << 2)

                         | ((raw & 0x02 ^ 0x02) << 2)
                         | ((raw & 0x02) << 1)
                         | ((raw & 0x01 ^ 0x01) << 1)
                         | ((raw & 0x01) << 0);
}

static inline int InsertedWrite(char *payload_w) {
  //unsigned start_t = 0;
  //unsigned end_t = 0;

  //start_t = __rdtsc();
  unsigned short crc = 0;
  payload_w[0] = 0x55;
  payload_w[2] = 0x7E;    // start
  payload_w[4] = 0x00;    // type
  payload_w[6] = 0x00;    // flag
  payload_w[8] = 80;      // lenght
  payload_w[170] = 0x00;  // clear CRC byte
  payload_w[172] = 0x00;  // clear CRC byte
  payload_w[174] = 0x7E;  // end
  for (int i = 0; i < 176; i += 2) {
    if (i >= 10 && i < 170) {
      // compute CRC
      crc = crc ^ (payload_w[i] << 8);
      for (int b = 0; b < 8; b++) {

        if (crc & 0x8000) {
          crc = (crc << 1) ^ 0x1021;
        } else {
          crc = (crc << 1);
        }
      }

    } else if (i == 170) {
      payload_w[i] = crc >> 8;
    } else if (i == 172) {
      payload_w[i] = crc & 0xFFFF;
    }
    MakeManchester(payload_w, i);
  }
  //end_t = __rdtsc();
  return crc;
}


static inline void LoadMessage(char *payload_w, const char *message) {
  for (int i = 0; i < 80; i++) {
    payload_w[(i + 5) * 2] = message[i];
  }
  ComputeCRC(payload_w);
  MakeManchester(payload_w);
}
