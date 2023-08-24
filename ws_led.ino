/**
 *
 * @file ws_led.ino
 * @author Bempah Dwomoh (you@domain.com) | Barnabas Nomo (barnabasnomo@gmail.com)
 * @brief Main code file for development of Smart Furniture Firmware
 * @version 0.1
 * @date 2023-07-30
 *
 * @copyright Copyright (c) 2023
 *
 */
/* Includes */
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Adafruit_NeoPixel.h>

/* Private define begin */
// Pin to use to send signals to WS2812B
// #define LED_PIN 6
#define LED_PIN 32
// Number of WS2812B LEDs attached to the Arduino
#define LED_COUNT 54

// WiFi Client Connection
const char *ssid = "";
const char *password = "";
/* Private define end*/

/* Private variables begin*/
// Setup HTTP server.
WebServer server(80);

// Setting up the NeoPixel library
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

unsigned long long lastUpdateLED;
long firstPixelHue = 0;
int ledDir = 1;

/* Private variables end */

/* Private function prototypes begin */
void setLedMode(int LEDMODE);
void rainbow(int wait);
void colorWipe(uint32_t color, int wait);
// http specific functions
void handleRoot();
void handleNotFound();
/* Private function prototypes end */

void setup()
{
  strip.begin();           // Initialize NeoPixel object
  strip.setBrightness(80); // Set BRIGHTNESS to about 4% (max = 255)
  strip.show();
  strip.clear(); // Set all pixel colors to 'off'
  delay(2000);
  colorWipe(strip.Color(0, 0, 255), 20);
  delay(5000);
}

void loop()
{

  // The first NeoPixel in a strand is #0, second is 1, all the way up
  // to the count of pixels minus one.

  /*
  for (int i = 0; i < LED_COUNT; i++)
  {
    // Set the i-th LED to pure green:
    strip.setPixelColor(i, 255, 255, 255);

    strip.show(); // Send the updated pixel colors to the hardware.

    delay(500); // Pause before next pass through loop
  }
  */
  rainbow(10);
}

/**
 * @brief handle all connections coming to default route '/'
 *
 */
void handleRoot()
{
  server.send(200, "text/plain", "awaiting user input");
}

/**
 * @brief Creates a rainbow sync across LED arra
 *
 * @param wait
 * @retval None
 */
void rainbow(int wait)
{
  int change = 256;
  if ((millis() - lastUpdateLED) >= wait)
  {
    lastUpdateLED = millis();
    strip.rainbow(firstPixelHue);
    strip.show();
    firstPixelHue += (ledDir * change);
    if ((firstPixelHue >= 65536) || (firstPixelHue <= 0))
    {
      ledDir = -ledDir;
    }
  }
}

/**
 * @brief creates an effect of light spreading linearly
 *
 * @param color
 * @param wait
 * @retval None
 */
void colorWipe(uint32_t color, int wait)
{
  int colorStep = 15;
  int startDiscardedValues = 255;
  uint32_t blendColor = color;
  int mid = LED_COUNT / 2;
  int opp = mid;
  for (int i = mid; i < LED_COUNT; i++)
  {
    if (startDiscardedValues > 0)
      startDiscardedValues -= colorStep;
    if (startDiscardedValues < 0)
      startDiscardedValues = 0;
    blendColor = strip.Color(startDiscardedValues, startDiscardedValues, 255);

    strip.setPixelColor(i, blendColor);
    if (opp > 0)
      opp--;
    strip.setPixelColor(opp, blendColor);
    strip.show();

    delay(wait);
  }
}
