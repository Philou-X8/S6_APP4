
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
  char payload[80];
  Serial.println(sizeof(size_t));

}

void loop() {
  // put your main code here, to run repeatedly:

  int mask = 0xFFFF;
  Serial.println(mask);
  Serial.println(mask >> 15);

  ComputeCRC();

  delay(5000);
}

//-----------------------------------------
//------------- Functions -----------------
//-----------------------------------------

void ComputeCRC(){
  byte payload[640 + 16];
  unsigned short mask = CRC_MASK;

  // The payload is 80 byte, add the CRC and you get 96 byte
  // That fits in an array of size_t[24]
  // 
  // The last 16 bits of index [23] contain the CRC buffer

  for(int i = 0; i < PAYLOAD_SIZE; i++){
    payload[i] = ( (7 * i) % 4 ) >> 1;
    byte temp = 0;
    temp = payload[i] ^ (mask >> 15);

    Serial.println(temp);
    delay(1000);
  }
  

}



//-----------------------------------------
//--------------- Tasks -------------------
//-----------------------------------------

void TaskTransmit(void *pvParameters){



}

void TaskReceive(void *pvParameters){



}



