#include <Adafruit_CircuitPlayground.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

// CONFIG
const bool WAIT_FOR_SERIAL = false;
const String APP_VERSION = "0.1";    // the version of this app
const String APP_NAME = "Bootstrap"; // the name of this app

// PINS
const byte AUDIBLE_OUTPUT_PIN = A0;

// NEOPIXELS
byte neoPixelBrightness = 2; // 0-255
const byte N_LEDS = 10;      // there 10 NeoPixel LEDs on the CPE

// DISPLAY
const bool USE_OLED = true;
const bool UPSIDE_DOWN_DISPLAY = false;
const bool DISPLAY_SHOULD_SLEEP = false;
const long SLEEP_DISPLAY_AFTER_DURATION = 9000;
const bool SHOW_ADAFRUIT_LOGO_ON_DISPLAY_STARTUP = false;

// BUTTON PINS ON OLED FEATHERWING
#define BUTTON_A 9
#define BUTTON_B 6
#define BUTTON_C 5

// INTERNAL
const byte OLED_RESET = 5;
Adafruit_SSD1306 display(OLED_RESET);
unsigned long timeDisplayStarted = 0;

void setup()
{
  CircuitPlayground.begin(neoPixelBrightness);
  pinMode(AUDIBLE_OUTPUT_PIN, OUTPUT);
  initSerialAndDisplay();
}

void loop()
{
  spinLEDs(false);
}

void spinLEDs(bool makeNoise)
{
  for (int i = 0; i < N_LEDS; i++)
  {
    CircuitPlayground.clearPixels();
    CircuitPlayground.setPixelColor(i, 0, 255, 160);
    if (makeNoise)
      digitalWrite(AUDIBLE_OUTPUT_PIN, HIGH);
    delayMicroseconds(1);
    if (makeNoise)
      digitalWrite(AUDIBLE_OUTPUT_PIN, LOW);
    delay(100);
  }
}

/*
 *
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * ************************************************
 * **********                         *************
 * **********     UTILITY METHODS     *************
 * **********                         *************
 * ************************************************
 */

void traceToDisplay(String msg)
{
  updateDisplay(msg);
}

void traceToSerial(String msg)
{
  Serial.println(String(millis()) + " :: " + msg);
}

void trace(String msg)
{
  traceToSerial(msg);
  updateDisplay(msg);
}

void initDisplay()
{
  if (!USE_OLED)
    return;
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // initialize with the I2C addr 0x3C (for the 128x32)
  if (UPSIDE_DOWN_DISPLAY)
  {
    display.setRotation(2);
  }
  if (SHOW_ADAFRUIT_LOGO_ON_DISPLAY_STARTUP)
  {
    display.display(); // show Adafruit logo
    delay(500);
  }
  // Clear the buffer.
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(4);
  display.display();
}

void updateDisplay(String msg)
{
  if (!USE_OLED)
    return;
  unsigned int msgLength = msg.length();
  byte textSize = msgLength <= 6 ? 4 : msgLength <= 7 ? 3 : msgLength <= 12 ? 2 : 1;
  display.setTextSize(textSize);
  drawTextToDisplay(msg);
}

void updateDisplayWithFixedSizeText(String msg, byte textSize)
{
  if (!USE_OLED)
    return;
  display.setTextSize(textSize);
  drawTextToDisplay(msg);
}

void drawTextToDisplay(String msg)
{
  timeDisplayStarted = millis();
  display.clearDisplay();
  byte cursorY = 0;
  display.setCursor(0, cursorY);
  display.clearDisplay();
  display.println(msg);
  display.display();
}

void clearDisplay()
{
  if (!USE_OLED)
    return;
  display.clearDisplay();
  display.display();
}

void checkForDisplaySleep()
{
  if (!USE_OLED)
    return;
  if (DISPLAY_SHOULD_SLEEP && millis() - timeDisplayStarted > SLEEP_DISPLAY_AFTER_DURATION)
  {
    clearDisplay();
  }
}

void flashSOS()
{
  int duration;
  int flashDurations[] = {100, 200, 300, 400, 300, 200};
  for (unsigned int i = 0; i < 6; i++)
  {
    duration = flashDurations[i];
    digitalWrite(LED_BUILTIN, HIGH);
    delay(duration);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }
}

void initSerialAndDisplay()
{
  if (!WAIT_FOR_SERIAL)
    delay(1000);
  initDisplay();
  Serial.begin(9600);
  if (WAIT_FOR_SERIAL)
  {
    traceToDisplay("Waiting for Serial");
    while (!Serial)
    {
      flashSOS();
    };
  }
  String startUpMsg = APP_NAME + "\nv" + APP_VERSION;
  traceToSerial(APP_NAME);
  updateDisplayWithFixedSizeText(startUpMsg, 2);
  delay(1200);
}