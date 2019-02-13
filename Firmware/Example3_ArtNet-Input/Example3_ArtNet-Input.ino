#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArtnetWifi.h>
#include <FastLED.h>

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

//Pin Definitions for Shield
#define TX 16
#define RX 17
#define EN 21
#define LED_DATA1 27
#define LED_DATA2 18
#define LED_DATA3 19
#define LED_CLOCK 5

//LED String Definitions
#define NUM_LEDS1 0
#define NUM_LEDS2 256
#define NUM_LEDS3 0
#define TOTAL_LEDS NUM_LEDS1 + NUM_LEDS2 + NUM_LEDS3
#define MAX_LEDS_PER_UNIVERSE 170
CRGB ledString1[NUM_LEDS1];
CRGB ledString2[NUM_LEDS2];
CRGB ledString3[NUM_LEDS3];

const int startUniverse = 0;
const int endUniverse = startUniverse + (TOTAL_LEDS / MAX_LEDS_PER_UNIVERSE);

boolean connectWifi(void)
{
  boolean state = true;
  WiFi.mode(WIFI_AP_STA);
  state = WiFi.softAP(ssid, password);
  return state;
}

//onDmxFrame is called every time we receive a DMX packet.
void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  sendFrame = 1;
  // set global brightness
  if (universe == 15)
  {
    FastLED.setBrightness(data[0]);
  }
  // read universe and put into the right part of the display buffer
  for (int i = 0; i < length / 3; i++)
  {
    int led = i + (universe - startUniverse) * (previousDataLength / 3); //Calculate which LED we will be writing to based on where we are in our data
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
  if (universe == endUniverse) //If we've received all our data, show it on the LED's
  {
    FastLED.show();
  }
}

void setup() {
  Serial.begin(115200);
  if (connectWifi())
  {
    Serial.println("Connected!");
  }

  //LED Initialization
  if (NUM_LEDS1 != 0)
  {
    FastLED.addLeds<WS2812, LED_DATA1, BGR>(ledString1, NUM_LEDS3);
  }
  if (NUM_LEDS2 != 0)
  {
    FastLED.addLeds<APA102, LED_DATA2, LED_CLOCK, BGR>(ledString2, NUM_LEDS2);
  }
  if (NUM_LEDS3 != 0)
  {
    FastLED.addLeds<APA102, LED_DATA3, LED_CLOCK, BGR>(ledString3, NUM_LEDS1);
  }
  artnet.begin();
  artnet.setArtDmxCallback(onDmxFrame);
}

void loop() {
  artnet.read();
}
