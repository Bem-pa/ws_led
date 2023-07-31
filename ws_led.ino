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
#include <Adafruit_NeoPixel.h>

/* Private define begin */
// Pin to use to send signals to WS2812B
#define LED_PIN 6

// Number of WS2812B LEDs attached to the Arduino
#define LED_COUNT 11
/* Private define end*/

/* Private variables begin*/
// Setting up the NeoPixel library
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

unsigned long long lastUpdateLED;
long firstPixelHue = 0;
int ledDir = 1;

/* Private variables end */

/* Private function prototypes begin */
void setLedMode(int LEDMODE);
void rainbow(int wait);
/* Private function prototypes end */

void setup()
{
  strip.begin();            // Initialize NeoPixel object
  strip.setBrightness(255); // Set BRIGHTNESS to about 4% (max = 255)
  strip.show();
  strip.clear(); // Set all pixel colors to 'off'
}

void loop()
{

  // The first NeoPixel in a strand is #0, second is 1, all the way up
  // to the count of pixels minus one.
  for (int i = 0; i < LED_COUNT; i++)
  {
    // Set the i-th LED to pure green:
    strip.setPixelColor(i, 255, 255, 255);

    strip.show(); // Send the updated pixel colors to the hardware.

    delay(500); // Pause before next pass through loop
  }
}

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