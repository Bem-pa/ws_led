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

#include "p_defaults.h"

/* Private define begin */
// Pin to use to send signals to WS2812B
// #define LED_PIN 6
#define LED_PIN 32
// Number of WS2812B LEDs attached to the Arduino
#define LED_COUNT 54

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
void handleUpdates();
/* Private function prototypes end */

void setup()
{
  strip.begin();           // Initialize NeoPixel object
  strip.setBrightness(80); // Set BRIGHTNESS to about 4% (max = 255)
  strip.show();
  strip.clear(); // Set all pixel colors to 'off'
  delay(2000);
  /* WiFi connection config begin */
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32"))
  {
    Serial.println("MDNS responder started");
  }

  server.onNotFound(handleNotFound); // 404
  server.on("/", handleRoot);        // root path
  server.on("/update", handleUpdates);
  server.begin();

  Serial.println("Server Started");
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
  server.handleClient();
  rainbow(10);
}

/**
 * @brief handle all connections coming to default route '/'
 *
 */
void handleRoot()
{
  String debug_message = (server.method() == HTTP_GET) ? "GET" : "POST";
  debug_message += "\t\t" + server.uri();
  server.send(200, "text/html", indexHtml);
}

/**
 * @brief 404 Error Handler
 *
 */
void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

/**
 * @brief Handle updates from client interface
 *
 */
void handleUpdates()
{

  // check params
  // int args = server.args();
  // for (uint8_t i = 0; i < args; i++)
  // {
  //   Serial.print(server.argName(i));
  //   Serial.print(": ");
  //   Serial.println(server.arg(i));
  // }
  // Read params
  if (server.hasArg("col"))
  {
    String col = server.arg("col");
    Serial.println(col);
    server.send(200, "text/plain", "Payload received");
  }
  else
  {
    Serial.println("no data");
    server.send(500, "text/plain", "Unable to process data");
  }
  String debug_message = (server.method() == HTTP_GET) ? "GET" : "POST";
  debug_message += "\t\t" + server.uri();
  Serial.println(debug_message);
  // work on debug string for routes later
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
