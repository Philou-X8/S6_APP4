
#include "esp_task_wdt.h"
#include "soc/gpio_struct.h"

#define CRC_MASK 0x1021  // CRC-16-CCITT

#define PAYLOAD_SIZE 640  // 80 byte * 8 bits = 640

#define TANSMIT_CORE 0
#define RECEIVE_CORE 0

#define SEND_PIN 25
#define READ_PIN 26

//hw_timer_t *timer = NULL;
//wdt_hal_context_t rtc_wdt_ctx = RWDT_HAL_CONTEXT_DEFAULT();


char payload[176] = { 0 };
void TaskReceive(void *pvParameters);
/*
void TaskTransmit(void *pvParameters);


void setup() {
  Serial.begin(115200);
  pinMode(READ_PIN, INPUT_PULLUP);
  pinMode(SEND_PIN, OUTPUT);
  //
  // Build hardcoded payload
  //

  //GPIO.out;

  xTaskCreate(
    TaskTransmit, "Send data thread",
    2048  // Stack size
    ,
    payload  // When no parameter is used, simply pass NULL
    ,
    1  // Priority
    ,
    NULL  // With task handle we will be able to manipulate with this task.
    //,
    //TANSMIT_CORE  // Core on which the task will run
  );

  xTaskCreatePinnedToCore(
    TaskReceive, "read data thread",
    2048  // Stack size
    ,
    NULL  // When no parameter is used, simply pass NULL
    ,
    2  // Priority
    ,
    NULL  // With task handle we will be able to manipulate with this task.
    ,
    1  // Core on which the task will run
  );

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

  //attachInterrupt(digitalPinToInterrupt(READ_PIN), TestInterrupt, CHANGE);
}

volatile int test_counter = 0;
int write_mode = 0;

void loop() {
  // put your main code here, to run repeatedly:
  //write_mode = !write_mode;
  //digitalRead(READ_PIN);
  //digitalWrite(SEND_PIN, write_mode);
  //Serial.println(test_counter);
  //int mask = 0xFFFF;
  //Serial.println(mask);
  //Serial.println(mask >> 15);

  //ComputeCRC();

  //delay(200);
}

void TestInterrupt() {
  test_counter++;
}
*/

//-----------------------------------------
//------------- Functions -----------------
//-----------------------------------------
/*
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
  //return crc;
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

static inline void LoadMessage(char *payload_w, char *message) {
  for (int i = 0; i < 80; i++) {
    payload_w[(i + 5) * 2] = message[i];
  }
  ComputeCRC(payload_w);
  MakeManchester(payload_w);
}
*/
//-----------------------------------------
//-------------- Interrupt ----------------
//-----------------------------------------

//byte read_arr[88];
//volatile byte read_buff;
//volatile byte read_index;
//volatile byte last_read_delay = 0; // "time" elapsed since last valid bit
//unsigned int read_min_delay = 10; // minimum amount of "time" to wait before a read becomes valid


/*
volatile int write_bit_index = 0;
void ARDUINO_ISR_ATTR SendBit() {

  int byte_index = (write_bit_index >> 3);  // last 3 bits are used to count the bit position
  int bit_index = (write_bit_index & 0x7);  // 0b0111
  // ((payload[byte_index] >> bit_index) & 0x1)
  
  //Serial.print("Sending byte [");
  //Serial.print(byte_index);
  //Serial.print("] and bit [");
  //Serial.print(bit_index);
  //Serial.print("] of value: ");
  //Serial.println((payload[byte_index] >> (8-bit_index)) & 0x1);
  

  if((payload[byte_index] >> (7-bit_index)) & 0x1){
    //Serial.println("trying to write 1:");
    GPIO.out_w1ts = (0x1 << SEND_PIN);
    //Serial.println(GPIO.out);
  }
  else{
    //Serial.println("trying to write 0:");
    GPIO.out_w1tc = (0x1 << SEND_PIN);
    //Serial.println(GPIO.out);
  }
  
  //digitalWrite(SEND_PIN, ((payload[byte_index] >> (7-bit_index)) & 0x1));

  write_bit_index += 1;
}

//-----------------------------------------
//--------------- Tasks -------------------
//-----------------------------------------

#define W_SPEED 500
void TaskTransmit(void *pvParameters) {
  char *a_payload = (char *)pvParameters;

  digitalWrite(SEND_PIN, 0);

  // Set timer frequency to 1Mhz
  timer = timerBegin(1000000);  //

  timerAttachInterrupt(timer, &SendBit);

  // Set alarm to call onTimer function every second (value in microseconds).
  // Repeat the alarm (third parameter) with unlimited count = 0 (fourth parameter).
  timerAlarm(timer, W_SPEED * 1000, false, 0);

  int isMessageReady = 0;
  int isTimerEnabled = 0;

  int sendIndex = 0;


  Serial.println("delay before sending message");
  delay(1000);

  while (1) {
    //wdt_hal_context_t rtc_wdt_ctx = RWDT_HAL_CONTEXT_DEFAULT();
    //wdt_hal_write_protect_disable(&rtc_wdt_ctx);
    //wdt_hal_feed(&rtc_wdt_ctx);
    //wdt_hal_write_protect_enable(&rtc_wdt_ctx);

    if (isMessageReady) {
      if (isTimerEnabled == 0) {
        timerRestart(timer);
        timerAlarm(timer, W_SPEED * 1000, true, 1408);  // 176 * 8 = 1408
        isTimerEnabled = 1;
      }


      if (write_bit_index >= 1408) {

        Serial.println("done sending message");
        isMessageReady = 0;
        timerAlarm(timer, W_SPEED * 1000, false, 0);
        isTimerEnabled = 0;
        write_bit_index = 0;

        delay(2000);
      }

    } else {
      char placeholder[80] = "This is a testing message. It doesnt mean anything, it just need to use 80 char";

      LoadMessage(a_payload, placeholder);

      Serial.println("message encoded, sending now");
      Serial.println((int)a_payload[0]);
      delay(500);
      isMessageReady = 1;
      sendIndex = 0;
      timerRestart(timer);
    }
    delay(1);  // prevent watchdog from freaking out
  }
}
*/



/*
volatile int callibrate_counter = 0;
volatile unsigned long calib_start = 0;
volatile unsigned long calib_end = 0;
volatile int read_priode = 0;
volatile int read_max_delay = 0;
volatile int reader_state = 0;
void InterruptIdle() {  // must call on rising edge

  if (callibrate_counter == 1) {
    calib_start = micros();
  } 
  else if (callibrate_counter == 3) {
    calib_end = micros();
    read_priode = (calib_end - calib_start);
    read_max_delay = (read_priode >> 3) + (read_priode >> 4); // 1/8 + 1/16 = 3/16
    Serial.print("read_max_delay = ");
    Serial.println(read_max_delay);
  } 
  else if (callibrate_counter == 4) {
    if(reader_state == 0){

      Serial.println("changing from state 0 to state 1");
      reader_state = 1;
    }
  }

  Serial.println(callibrate_counter);
  callibrate_counter += 1;
}

volatile unsigned long last_bit_t = 0;
volatile unsigned int read_in_buffer = 0;
volatile int read_bit_counter;
volatile int read_buff_size;
void InterruptRise() {

    //Serial.print("rising edge, delay = ");
    //Serial.println((micros() - last_bit_t));

  if ((micros() - last_bit_t) > read_max_delay) {
    read_in_buffer = (read_in_buffer << 1) | 0x1;  // insert a 1 at the LSB
    read_bit_counter++;
    read_buff_size++;
    Serial.print("detect 1");
    last_bit_t = micros();
  }
}
void InterruptFall() {
  int pin_state = digitalRead(READ_PIN);
  if(digitalRead(READ_PIN)){

    //Serial.print("rising edge, delay = ");
    //Serial.println((micros() - last_bit_t));

  if ((micros() - last_bit_t) > read_max_delay) {
    read_in_buffer = (read_in_buffer << 1) | 0x1;  // insert a 1 at the LSB
    read_bit_counter++;
    read_buff_size++;
    //Serial.println("detect 1");
    last_bit_t = micros();
  }


  }
  else
  {

    //Serial.print("falling edge, delay = ");
    //Serial.println((micros() - last_bit_t));

  if ((micros() - last_bit_t) > read_max_delay) {
    read_in_buffer = (read_in_buffer << 1);  // insert a 0 at the LSB
    read_bit_counter++;
    read_buff_size++;
    //Serial.println("detect 0");
    last_bit_t = micros();
  }

  }
}



void TaskReceive(void *pvParameters) {

  
  Serial.println("TaskReceive tread");

  // Set timer frequency to 1Mhz
  //timer = timerBegin(1000000);

  unsigned long start_time = micros();


  int handled_bit_counter = 0;

  char message_buff[88] = { 0 };
  int message_buff_index = 0;
  

  attachInterrupt(digitalPinToInterrupt(READ_PIN), InterruptIdle, RISING);

  Serial.println("TaskReceive tread, ready to read");

  //start_time = micros();
  while (1) {
    delay(1);

    if (reader_state == 0) {
      if(callibrate_counter > 4){
        //Serial.println(callibrate_counter);
      }
    } else if (reader_state == 1) {
      // detach interrupt
      detachInterrupt(digitalPinToInterrupt(READ_PIN));

      read_in_buffer = 0;
      //message_buff = ""; // fix resetting the read buffer
      message_buff_index = 0;

      // attach new interrupt
      //attachInterrupt(digitalPinToInterrupt(READ_PIN), InterruptRise, RISING);
      //attachInterrupt(digitalPinToInterrupt(READ_PIN), InterruptFall, FALLING);
      attachInterrupt(digitalPinToInterrupt(READ_PIN), InterruptFall, CHANGE);

      last_bit_t = micros();

      reader_state = 2;
    } 
    else if (reader_state == 2) {
      for (int i = (read_buff_size - 3); i >= 0; i--) {

        if (((read_in_buffer >> i) & 0x7) == 0x6) {  // search for the first instance of '110' which mark the end of the Start flag
          read_in_buffer = read_in_buffer % (0x1 << i);
          read_buff_size = i;
          message_buff_index = 0;
          reader_state = 3;  // change state
          Serial.println("HEADER FOUND");
          break;
        }
      }

      if (read_buff_size >= 16) {  // start flag not found
        Serial.println("ERROR: START FLAG NOT FOUND IN MESSAGE");
        // Try to not fall in a death loop, idk
      }
    } 
    else if (reader_state == 3) {  // just read for now, we'll parse later
      if (read_buff_size >= 8) {
        int temp = read_in_buffer >> (read_buff_size - 8);
        message_buff[message_buff_index] = (char)temp;

        Serial.print("recieved character: ");
        Serial.println((char)temp);

        //read_in_buffer = read_in_buffer ^ (temp << (read_buff_size - 8));
        read_in_buffer = read_in_buffer % (0x1 << (read_buff_size - 8));  // keep only the last  bits
        read_buff_size -= 8;

        message_buff_index++;
      }

      if (message_buff_index >= 82){ // we should have all the message
        reader_state = 4;
      } 
    }
    else if (reader_state == 4){ // parse the message and shit
      // stop reading the pin
      detachInterrupt(digitalPinToInterrupt(READ_PIN));

      Serial.println("Transmittion complete, here's the raw message:");
      Serial.println(payload);

      // do the parsing 
      delay(1000);

      // go to back to standby mode
    }


  }
}
*/