
#include "esp_task_wdt.h"
#include "soc/gpio_struct.h"

#define CRC_MASK 0x1021  // CRC-16-CCITT

#define TANSMIT_CORE 0
#define RECEIVE_CORE 1

#define SEND_PIN 25
#define READ_PIN 26

//#define WRITE_SPEED 500 * 1000

hw_timer_t *timer = NULL;
//wdt_hal_context_t rtc_wdt_ctx = RWDT_HAL_CONTEXT_DEFAULT();

void TaskTransmit(void *pvParameters);
void TaskReceive(void *pvParameters);

char g_payload[176] = { 0 };

void setup() {
  Serial.begin(115200);
  pinMode(READ_PIN, INPUT_PULLUP);
  pinMode(SEND_PIN, OUTPUT);

  g_payload[0] = 0x55;
  g_payload[2] = 0x7E;    // start
  g_payload[4] = 0x00;    // type
  g_payload[6] = 0x00;    // flag
  g_payload[8] = 80;      // lenght
  g_payload[170] = 0x00;  // clear CRC byte
  g_payload[172] = 0x00;  // clear CRC byte
  g_payload[174] = 0x7E;  // end
  for (int i = 0; i < 176; i += 2) {
    if (i >= 10 && i < 174) {
      continue;
    } else {
      MakeManchester(g_payload, i);
    }
  }

  // Task function
  // nickname
  // Stack size
  // Params. When no parameter is used, simply pass NULL
  // Priority
  // With task handle we will be able to manipulate with this task.
  // Core on which the task will run
  xTaskCreatePinnedToCore(TaskTransmit, "Send data thread", 2048, g_payload, 1, NULL, TANSMIT_CORE);
  xTaskCreatePinnedToCore(TaskReceive, "read data thread", 2048, NULL, 2, NULL, RECEIVE_CORE);

}

void loop(){

}
