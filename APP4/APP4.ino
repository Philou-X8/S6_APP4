
#define CRC_MASK 0x1021  // CRC-16-CCITT

#define PAYLOAD_SIZE 640  // 80 byte * 8 bits = 640

#define TANSMIT_CORE 0
#define RECEIVE_CORE 0

void TaskTransmit(void *pvParameters);
void TaskReceive(void *pvParameters);

char* payload[176] = { 0 };

void setup() {
  Serial.begin(115200);

  //
  // Build hardcoded payload
  //

  /*
  xTaskCreatePinnedToCore(
    TaskTransmit, "Send data thread"
    , 
    2048  // Stack size
    ,
    NULL  // When no parameter is used, simply pass NULL
    ,
    1  // Priority
    ,
    NULL  // With task handle we will be able to manipulate with this task.
    ,
    TANSMIT_CORE  // Core on which the task will run
  );
  */
  payload[0] = 0x55;
  payload[2] = 0x7E;    // start
  payload[4] = 0x00;    // type
  payload[6] = 0x00;    // flag
  payload[8] = 80;      // lenght
  payload[170] = 0x00;  // clear CRC byte
  payload[172] = 0x00;  // clear CRC byte
  payload[174] = 0x7E;  // end
  for (int i = 0; i < 176; i += 2) {
    if (i >= 10 && i < 174) {
      continue;
    } else {
      MakeManchester(payload, i);
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:

  int mask = 0xFFFF;
  Serial.println(mask);
  Serial.println(mask >> 15);

  //ComputeCRC();

  delay(5000);
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
        crc = (crc << 1) ^ 0x1021;
      } else {
        crc = (crc << 1);
      }
    }
  }
  //std::cout << "\nmake CRC: " << (crc) << std::endl;

  payload_w[end_i + 0] = crc >> 8;
  payload_w[end_i + 2] = crc & 0xFFFF;
  return crc;
}

static inline int CheckCRC(char *payload) {
  const int start_i = 10;  // data payload start at byte 5
  const int end_i = 170;   // data payload end at byte 5
  unsigned short crc = 0;

  for (int i = start_i; i < (end_i + 4); i += 2) {
    crc = crc ^ (payload[i] << 8);

    for (int b = 0; b < 8; b++) {

      if (crc & 0x8000) {
        crc = (crc << 1) ^ 0x1021;
      } else {
        crc = (crc << 1);
      }
    }
  }
  //std::cout << "\ncheck CRC: " << (crc) << std::endl;
  return crc;  // (crc > 0) if there's an error
}


static inline void MakeManchester(char *payload_w) {
  const int start_i = 10;  // data payload start at byte 5*2=10
  const int end_i = 168;   // data payload end at byte (85-1)*2=168
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

static inline void LoadMessage(char *payload_w, char *message){
  for(int i = 0; i<80; i++){
    payload_w[(i+5)*2] = message[i];
  }
  ComputeCRC(payload_w);
  MakeManchester(payload_w);
}

//-----------------------------------------
//--------------- Tasks -------------------
//-----------------------------------------

void TaskTransmit(void *pvParameters) {
}

void TaskReceive(void *pvParameters) {
}
