#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArtnetWifi.h>
#include <FastLED.h>
#include <LXESP32DMX.h> //https://github.com/claudeheintz/LXESP32DMX

//Wifi settings
char ssid[] = "myDMX";
char password[] = "artnetnode";
IPAddress local_IP(192, 168, 1, 184);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// Artnet settings
ArtnetWifi artnet;
const int startUniverse = 0;
const int endUniverse = 1;

bool sendFrame = 1;
int previousDataLength = 0;

//Pin Definitions for Shield
#define RX 16
#define TX 17
#define EN 21
#define LED_DATA1 27
#define LED_DATA2 18
#define LED_DATA3 19
#define LED_CLOCK 5

//LED String Definitions
#define NUM_LEDS1 0
#define NUM_LEDS2 192
#define NUM_LEDS3 0
#define TOTAL_LEDS NUM_LEDS1 + NUM_LEDS2 + NUM_LEDS3
CRGB ledString1[NUM_LEDS1];
CRGB ledString2[NUM_LEDS2];
CRGB ledString3[NUM_LEDS3];

uint8_t hue = 0;

boolean connectWifi(void)
{
  boolean state = true;
  WiFi.mode(WIFI_AP_STA);
  state = WiFi.softAP(ssid, password);
  return state;
}

void beginXLROut()
{
  pinMode(EN, OUTPUT);
  digitalWrite(EN, HIGH);

  pinMode(TX, OUTPUT);
  ESP32DMX.startOutput(TX);
}

//Beginning XLR Input will mean that the output will simply be a throughput
void beginXLRIn()
{
  pinMode(EN, OUTPUT);
  digitalWrite(EN, LOW);

  pinMode(RX, INPUT);
  ESP32DMX.startInput(RX);
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  sendFrame = 1;
  // set brightness of the whole strip
  if (universe == 15)
  {
    FastLED.setBrightness(data[0]);
  }
  // read universe and put into the right part of the display buffer
  for (int i = 0; i < length / 3; i++)
  {
    int led = i + (universe - startUniverse) * (previousDataLength / 3);
    if (led < NUM_LEDS1)
    {
      ledString1[led] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
    }
    else if (led < NUM_LEDS2 + NUM_LEDS1 && led >= NUM_LEDS1)
    {
      ledString2[led] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
    }
    else if (led < TOTAL_LEDS && led >= NUM_LEDS1 + NUM_LEDS2)
    {
      ledString3[led] = CRGB(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
    }
  }
  previousDataLength = length;
  if (universe == endUniverse)
  {
    FastLED.show();
  }
}

void copyDMXToOutput(void) {
  xSemaphoreTake( ESP32DMX.lxDataLock, portMAX_DELAY );
  for (int i = 1; i < DMX_MAX_FRAME; i++) {
    ESP32DMX.setSlot(i , dmxbuffer[i]);
  }
  xSemaphoreGive( ESP32DMX.lxDataLock );
}

void artnetToXLR (uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  //add in a check for universe number, have a dedicated XLR universe since our device can handle multiple universes
  uint8_t dmxBuffer[DMX_MAX_FRAME];
  sendFrame = 1;
  for (int i = 0; i < length; i++)
  {
    dmxBuffer[i] = data[i];
  }
  previousDataLength = length;
}

void writeChannel(uint16_t channel, uint8_t value)
{
    uint8_t dmxBuffer[DMX_MAX_FRAME];
    dmxBuffer[channel] = value;
}

void setup() {
  Serial.begin(115200);
  if (connectWifi())
  {
    Serial.println("Connected!");
    //Serial.println(WiFi.softAPIP());
  }

  //LED Initialization
  if (NUM_LEDS1 != 0)
  {
    FastLED.addLeds<WS2812, LED_DATA1, BGR>(ledString3, NUM_LEDS3);
  }
  if (NUM_LEDS2 != 0)
  {
    FastLED.addLeds<APA102, LED_DATA2, LED_CLOCK, BGR>(ledString2, NUM_LEDS2);
  }
  if (NUM_LEDS3 != 0)
  {
    FastLED.addLeds<APA102, LED_DATA3, LED_CLOCK, BGR>(ledString1, NUM_LEDS1);
  }
  FastLED.setBrightness(32);
  artnet.begin();
  artnet.setArtDmxCallback(onDmxFrame);
}

void loop() {
  artnet.read();
}
