#include <Adafruit_CircuitPlayground.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

// CONFIG
const bool WAIT_FOR_SERIAL = false;  // pause app until serial monitor is connected
const String APP_VERSION = "0.2";    // the version of this app
const String APP_NAME = "Bootstrap"; // the name of this app
long neoPixelColour = 0x00FF9D;

// PINS
const byte AUDIBLE_OUTPUT_PIN = A0;

// NEOPIXELS
byte neoPixelBrightness = 2; // 0-255
const byte N_LEDS = 10;      // there 10 NeoPixel LEDs on the CPE
unsigned int activeNeoPixel = 0;

// DISPLAY
const bool USE_OLED = true;
const bool UPSIDE_DOWN_DISPLAY = false;
const bool DISPLAY_SHOULD_SLEEP = false;
const long SLEEP_DISPLAY_AFTER_DURATION = 9000;
const bool SHOW_ADAFRUIT_LOGO_ON_DISPLAY_STARTUP = false;

// INTERNAL
const byte OLED_RESET = 5;
Adafruit_SSD1306 display(OLED_RESET);
unsigned long timeDisplayStarted = 0;

void setup()
{
  CircuitPlayground.begin(neoPixelBrightness);
  pinMode(AUDIBLE_OUTPUT_PIN, OUTPUT);
  initSerialAndDisplay();
  randomizeSeed();
}

void loop()
{
  spinLEDs(600);
  checkForLeftButtonRelease();
  // right button disabled for use when using I2C
  checkForDisplaySleep();
}

void handleLeftButtonRelease()
{
  trace("handleLeftButtonRelease");
  neoPixelColour = random(0x000000, 0xFFFFFF);
  trace(String(neoPixelColour, HEX));
}

void spinLEDs(unsigned int ledDuration)
{
  bool makeNoise = CircuitPlayground.slideSwitch();
  int idx = floor(millis() / ledDuration);
  idx %= N_LEDS;
  if (idx != activeNeoPixel)
  {
    CircuitPlayground.clearPixels();
    CircuitPlayground.setPixelColor(idx, neoPixelColour);
    if (makeNoise)
    {
      digitalWrite(AUDIBLE_OUTPUT_PIN, HIGH);
      delayMicroseconds(1);
      digitalWrite(AUDIBLE_OUTPUT_PIN, LOW);
    }
    activeNeoPixel = idx;
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

void checkForLeftButtonRelease()
{
  if (CircuitPlayground.leftButton())
  {
    trace("waiting for leftButton release");
    while (CircuitPlayground.leftButton())
      ;
    trace("leftButton has been release");
    handleLeftButtonRelease();
  }
}

void randomizeSeed()
{
  byte pin = A3;
  analogReadResolution(12);
  pinMode(pin, INPUT);
  digitalWrite(pin, LOW); // disable internal pullup
  delay(150);
  unsigned int floatingInput = analogRead(pin);
  digitalWrite(pin, HIGH); // enable internal pullup
  randomSeed(floatingInput);
  analogReadResolution(10);
}

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
  if (DISPLAY_SHOULD_SLEEP && timeDisplayStarted > 0 && millis() - timeDisplayStarted > SLEEP_DISPLAY_AFTER_DURATION)
  {
    clearDisplay();
    timeDisplayStarted = 0;
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
  traceToSerial(startUpMsg);
  updateDisplayWithFixedSizeText(startUpMsg, 2);
  delay(1200);
}
