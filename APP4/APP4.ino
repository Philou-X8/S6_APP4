
#define CRC_MASK 0x1021 // CRC-16-CCITT

#define PAYLOAD_SIZE 640 // 80 byte * 8 bits = 640

#define TANSMIT_CORE 0
#define RECEIVE_CORE 0

void TaskTransmit(void *pvParameters);
void TaskReceive(void *pvParameters);


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

  Serial.println("payload test:\n");
  byte payload[174]; // 87 * 2 = 174
  ComputeCRC(payload);

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

static inline void ComputeCRC(byte* payload_w){
  const int start_i = 10; // data payload start at byte 5*2=10
  const int end_i = 170; // data payload end at byte 85*2=170
  unsigned short crc = 0;

  for (int i = start_i; i < end_i; i += 2){
    crc = crc ^ (payload_w[i] << 8);

    for (int b = 0; b < 8; b++){

      if (crc & 0x8000) {
        crc = (crc << 1) ^ 0x1201;
      }
      else {
        crc = (crc << 1);
      }
      
    }
  }
  payload_w[end_i + 0] = crc >> 8;
  payload_w[end_i + 2] = crc & 0xFFFF;
}

static inline int CheckCRC(byte* payload){
  const int start_i = 5; // data payload start at byte 5
  const int end_i = 85; // data payload end at byte 5
  unsigned short crc = 0;

  for (int i = start_i; i < (end_i+2); i++){
    crc = crc ^ (payload[i] << 8);

    for (int b = 0; b < 8; b++){

      if (crc & 0x8000) {
        crc = (crc << 1) ^ 0x1021;
      }
      else {
        crc = (crc << 1);
      }
      
    }
  }
  
  return crc; // (crc > 0) if there's an error
}

void MakeManchester(byte* payload_w){
  const int start_i = 10; // data payload start at byte 5*2=10
  const int end_i = 170; // data payload end at byte 85*2=170
  
  const int crc_end_i = 172; // data payload end at byte 85*2=170

  byte prev_bit = payload_w[start_i-1];
  byte read_buffer = 0;
  byte write_buffer = 0;
  for(int i = start_i; i < crc_end_i; i += 2){
    read_buffer = payload_w[i];

    payload_w[i] = (read_buffer ^ 0x80)
    | ((read_buffer >> 1) & 0x40)
    | ((read_buffer >> 1) ^ 0x20)
    | ((read_buffer >> 2) & 0x10)

    | ((read_buffer >> 2) ^ 0x02)
    | ((read_buffer >> 3) & 0x01)
    | ((read_buffer >> 3) ^ 0x02)
    | ((read_buffer >> 4) & 0x01);

    
    payload_w[i+1] = ((read_buffer << 4) ^ 0x80)
    | ((read_buffer << 3) & 0x40)
    | ((read_buffer << 3) ^ 0x20)
    | ((read_buffer << 2) & 0x10)

    | ((read_buffer << 2) ^ 0x02)
    | ((read_buffer << 1) & 0x01)
    | ((read_buffer << 1) ^ 0x02)
    | ((read_buffer << 0) & 0x01);

    for (int b = 8; b > 0; b--){
      // payload_w[i + (b>>4)] move the index to (i+1) once we're past the 4th bit
      payload_w[i + int(b>4)] = (read_buffer & (0x1<<b)) ? (payload_w[i + int(b>4)] & (0x1)) : (payload_w[i + int(b>4)] & (0x2));
    }
    
  }
}


//-----------------------------------------
//--------------- Tasks -------------------
//-----------------------------------------

void TaskTransmit(void *pvParameters){



}

void TaskReceive(void *pvParameters){



}



