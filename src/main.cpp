// Importing necessary libraries
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_NeoPixel.h>
#include "SPIFFS.h"

#include "config/config.h"

#define DATA_PIN 19
#define NUM_LEDS 300
#define BRIGHTNESS_UP 1
#define BRIGHTNESS_DOWN 2
#define CHANGE_COLOR 39
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, DATA_PIN, NEO_GRBW + NEO_KHZ800);

// Setting network credentials
const char *ssid = CONFIG.WIFI_SSID;
const char *password = CONFIG.WIFI_PASSWORD;

bool terminate = false;

void rainbow(int wait)
{
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this loop:
  for (long firstPixelHue = 0; firstPixelHue < 5 * 65536 && !terminate; firstPixelHue += 256)
  {
    // Serial.print("TERMINATE? ");
    // Serial.println(terminate);

    // strip.rainbow() can take a single argument (first pixel hue) or
    // optionally a few extras: number of rainbow repetitions (default 1),
    // saturation and value (brightness) (both 0-255, similar to the
    // ColorHSV() function, default 255), and a true/false flag for whether
    // to apply gamma correction to provide 'truer' colors (default true).
    strip.rainbow(firstPixelHue);
    // Above line is equivalent to:
    // strip.rainbow(firstPixelHue, 1, 255, 255, true);
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

void theaterChaseRainbow(int wait)
{
  int firstPixelHue = 0; // First pixel starts at red (hue 0)
  for (int a = 0; a < 30 && !terminate; a++)
  { // Repeat 30 times...
    for (int b = 0; b < 3 && !terminate; b++)
    {                //  'b' counts from 0 to 2...
      strip.clear(); //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for (int c = b; c < strip.numPixels() && !terminate; c += 3)
      {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int hue = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color);                       // Set pixel 'c' to value 'color'
      }
      strip.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}

// Creating a AsyncWebServer object
AsyncWebServer server(80);

volatile unsigned int anim = 0;

void setup()
{
  // Serial port for debugging purposes
  Serial.begin(115200);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  if (!SPIFFS.begin("/"))
  {
    Serial.println("SPIFFS init failed");
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi");
  }

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  server.serveStatic("/", SPIFFS, "/app/").setDefaultFile("index.html");

  // Start server
  server.begin();
}

void loop()
{
  switch (anim)
  {
  case 0:
    Serial.println("Rainbow");
    rainbow(10); // Flowing rainbow cycle along the whole strip
    terminate = false;

    break;
  case 1:
    Serial.println("Theater");
    theaterChaseRainbow(50); // Rainbow-enhanced theaterChase variant
    terminate = false;

    break;

  default:
    break;
  }
}