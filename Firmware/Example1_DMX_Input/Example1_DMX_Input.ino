#include <LXESP32DMX.h>
#include "esp_task_wdt.h"

#define EN 21
#define RX 17

// the addresses of the slots to observe
int test_slotA = 0;
int test_slotB = 4;
int test_slotC = 2;

// the levels of those slots
uint8_t test_levelA = 0;
uint8_t test_levelB = 0;
uint8_t test_levelC = 0;

//pins for PWM output
//uint8_t led_pinA = 5;
//uint8_t led_pinB = 18;
//uint8_t led_pinC = 19;

//ledc channels (set to zero to disable)
uint8_t led_channelA = 0;
uint8_t led_channelB = 1;
uint8_t led_channelC = 2;

/************************************************************************
  attach a pin to a channel and configure PWM output
*************************************************************************/
/*void setupPWMChannel(uint8_t pin, uint8_t channel) {
  if ( channel ) {
    ledcAttachPin(pin, channel);
    ledcSetup(channel, 12000, 16); // 12 kHz PWM, 8-bit resolution
  }
  }*/

/************************************************************************
  gamma corrected write to a PWM channel
*************************************************************************/
/*void gammaCorrectedWrite(uint8_t channel, uint8_t level) {
  if ( channel ) {
    ledcWrite(channel, level*level);
  }
  }*/

/************************************************************************
  callback for when DMX frame is received
  Note:  called from receive task

  Checks to see if the level of the designated slot has changed
  and prints the new level to the serial monitor.  If a PWM channel is assigned,
  it also sets the output level.

*************************************************************************/

void receiveCallback(int slots) {
  Serial.println("CALLBACK");
  xSemaphoreTake( ESP32DMX.lxDataLock, portMAX_DELAY );
  ESP32DMX.printReceivedData();
  xSemaphoreGive( ESP32DMX.lxDataLock );

}

/************************************************************************
  setup
*************************************************************************/
void setup() {
  Serial.begin(115200);
  Serial.print("setup");

  ESP32DMX.setDirectionPin(EN);
  digitalWrite(EN, LOW);

  //setupPWMChannel(led_pinA, led_channelA);
  //setupPWMChannel(led_pinB, led_channelB);
  //setupPWMChannel(led_pinC, led_channelC);

  pinMode(RX, INPUT);
  ESP32DMX.startInput(RX);
  ESP32DMX.setDataReceivedCallback(receiveCallback);
  Serial.println("setup complete");
}


/************************************************************************
  main loop just idles
  vTaskDelay is called to prevent wdt timeout
*************************************************************************/

void loop() {
  delay(25);
}
