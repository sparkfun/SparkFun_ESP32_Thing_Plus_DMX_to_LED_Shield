#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArtnetWifi.h>
#include <FastLED.h>
#include <LXESP32DMX.h>
#include "esp_task_wdt.h"

//Wifi settings
char ssid[] = "myDMX";
char password[] = "artnetnode";
IPAddress local_IP(192, 168, 1, 184);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// Artnet settings
ArtnetWifi artnet;

bool sendFrame = 1;
int previousDataLength = 0;
uint8_t level;
uint8_t dmxBuffer[DMX_MAX_FRAME];

//Pin Definitions for Shield
#define TX 16
#define RX 17
#define EN 21

const int xlrUniverse = 2;

//Creates a stand alone access point to connect to
boolean connectWifi(void)
{
  boolean state = true;
  WiFi.mode(WIFI_AP_STA);
  state = WiFi.softAP(ssid, password);
  return state;
}

//Writes our DMX buffer to the output
void writeDMXToOutput(void) {
  xSemaphoreTake( ESP32DMX.lxDataLock, portMAX_DELAY );
  for (int i = 1; i < DMX_MAX_FRAME; i++) {
      ESP32DMX.setSlot(i , dmxBuffer[i]);
   }
   xSemaphoreGive( ESP32DMX.lxDataLock );
}

//onDmxFrame is called every time we receive a DMX packet.
void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  sendFrame = 1;
  // If the universe is our XLR universe, read it into our dmxBuffer
  if (universe == xlrUniverse)
  {
    for (int i = 0; i < length; i++)
    {
      dmxBuffer[i] = data[i];
    }
    previousDataLength = length;
    writeDMXToOutput(); //Write out all of our values on our XLR Output
  }
}

void setup() {
  Serial.begin(115200);
  if (connectWifi())
  {
    Serial.println("Connected!");
  }

  pinMode(EN, OUTPUT);
  digitalWrite(EN, HIGH);
  pinMode(TX, OUTPUT);
  ESP32DMX.startOutput(TX);

  artnet.begin();
  artnet.setArtDmxCallback(onDmxFrame);
}

void loop() {
  artnet.read();
  esp_task_wdt_feed();
  vTaskDelay(100);
}
