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
#define LED_MODE_SOLID 0x03
#define LED_MODE_GRADIENT 0x04
#define LED_MODE_BREATHING 0x05
/* Private define end*/

/* Private variables begin*/
// Setup HTTP server.
WebServer server(80);

// Setting up the NeoPixel library
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

unsigned long long lastUpdateLED;
long firstPixelHue = 0;
int ledDir = 1;

// Static IP config
IPAddress local_IP(192, 168, 137, 173);
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

/* Private variables end */

/* Private function prototypes begin */
void setLedMode(int LEDMODE);
void rainbow(int wait);
void colorWipe(uint32_t color, int wait);
void setGradient(uint32_t colors[]);
// http specific functions
void handleRoot();
void handleNotFound();
void handleUpdates();
/* Private function prototypes end */

void setup()
{
  strip.begin();           // Initialize NeoPixel object
  strip.setBrightness(20); // Set BRIGHTNESS to about 4% (max = 255)
  strip.show();
  strip.clear(); // Set all pixel colors to 'off'
  delay(1000);
  /* WiFi connection config begin */
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  // Create static IP address
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
  {
    Serial.println("STA Failed to configure");
  }
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
  // rainbow(10);
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

  uint32_t decodedCommandData;
  String mode;
  int cmd_mode = LED_MODE_SOLID;
  // Read params
  if (server.hasArg("mode"))
  {
    String tmp = server.arg("mode");
    if (tmp == "0x04")
    {
      cmd_mode = LED_MODE_GRADIENT;
    }
  }
  switch (cmd_mode)
  {
  case LED_MODE_GRADIENT:
    if (server.hasArg("len"))
    {
      String len_txt = server.arg("len");
      int len;
      const char *c_temp = len_txt.c_str();
      len = (int)strtol(c_temp, NULL, 16);
      uint32_t gradientColors[len];
      for (int i = 0; i < len; i++)
      {
        String placeholder = "col";
        if (server.hasArg(placeholder + String(i)))
        {
          String colorTxt = server.arg(placeholder + String(i));
          const char *colorParser = colorTxt.c_str();
          uint32_t currentColor = (uint32_t)strtol(colorParser, NULL, 16);
          gradientColors[i] = currentColor;
          Serial.println(currentColor, HEX);
        }
      }
      setGradient(gradientColors, len);
      server.send(200, "text/plain", "Ok");
    }
    else
    {
      server.send(500, "text/plain", "Expected 'len' Parameter");
    }

    break;
  case LED_MODE_SOLID:
    if (server.hasArg("col"))
    {
      String commandData = "";
      commandData = server.arg("col");
      Serial.print(commandData);
      const char *c_temp = commandData.c_str();
      long long temp = strtoll(c_temp, NULL, 16);
      int b = temp & 0xFF;
      // decoded
      temp = temp >> 8;
      int g = temp & 0xFF;
      temp = temp >> 8;
      int r = temp & 0xFF;
      uint32_t parsedColor = strip.Color(r, g, b);
      Serial.print(" ");
      Serial.println(parsedColor, HEX);
      for (int i = 0; i < LED_COUNT; i++)
      {
        strip.setPixelColor(i, parsedColor);
      }
      server.send(200, "text/plain", "Ok");
      strip.show();
    }
    else
    {
      Serial.println("no data");
      server.send(500, "text/plain", "Unable to process data");
    }
    break;

  default:
    Serial.println("no data");
    server.send(500, "text/plain", "Unable to process data");
    break;
  }

  String debug_message = (server.method() == HTTP_GET) ? "GET" : "POST";
  debug_message += "\t\t" + server.uri();
  Serial.println(debug_message);
}

/**
 * @brief Creates a rainbow sync across LED array
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

void setGradient(uint32_t colors[], int len)
{
  // int len = sizeof(colors) / sizeof(uint32_t);
  int mid = LED_COUNT / 2;
  int opp = mid;
  int steps = LED_COUNT / len / 2;
  // eg len = 4
  for (int i = mid; i < LED_COUNT; i++)
  {
    // int colorSelector = len - 1 - ((LED_COUNT - i) % len);
    int colorSelector = (i - mid) / 6;
    if (colorSelector > len - 1)
    {
      colorSelector = len - 1;
    }
    strip.setPixelColor(i, colors[colorSelector]);
    if (opp > 0)
      opp--;
    strip.setPixelColor(opp, colors[colorSelector]);
    strip.show();
    delay(20);
  }
}