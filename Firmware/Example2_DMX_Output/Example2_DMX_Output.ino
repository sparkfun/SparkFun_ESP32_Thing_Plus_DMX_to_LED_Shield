#include <LXESP32DMX.h>
#include "esp_task_wdt.h"

uint8_t level = 0;
uint8_t dmxBuffer[DMX_MAX_FRAME];

//Pin Definitions for Shield
#define TX 16
#define RX 17
#define EN 21

//Writes our DMX buffer to the output
void writeDMXToOutput(void) {
  xSemaphoreTake( ESP32DMX.lxDataLock, portMAX_DELAY );
  for (int i = 1; i < DMX_MAX_FRAME; i++) {
    ESP32DMX.setSlot(i , dmxBuffer[i]);
  }
  xSemaphoreGive( ESP32DMX.lxDataLock );
}

void setup() {
  pinMode(EN, OUTPUT);
  digitalWrite(EN, HIGH);
  pinMode(TX, OUTPUT);
  ESP32DMX.startOutput(TX);
}

void loop() {
  for (int channel = 1; channel < 512; channel++)
  {
    dmxBuffer[channel] = channel;
  }
  //level++;
  writeDMXToOutput();
  esp_task_wdt_feed();
  vTaskDelay(100);
}
