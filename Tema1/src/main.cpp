#include <Arduino.h>

// Pin Definitions
#define PIN_BUTTON_STOP 2
#define PIN_BUTTON_START 3
#define PIN_RGB_LED_GREEN 5
#define PIN_RGB_LED_BLUE 6
#define PIN_RGB_LED_RED 4
#define PIN_LED_100 7
#define PIN_LED_75 8
#define PIN_LED_50 9
#define PIN_LED_25 10

// Time Constants (in milliseconds)
#define CHARGING_TIME 12000
#define FORCE_STOP_TIME 1000
#define LED_BLINK_DURATION 4000
#define DEBOUNCE_DELAY 50

// State Variables
int startButtonState = HIGH;
int lastStartButtonState = HIGH;
int stopButtonState = HIGH;
int lastStopButtonState = HIGH;
long stopButtonPressTime = 0;
unsigned long lastStartDebounceTime = 0;
unsigned long lastStopDebounceTime = 0;
long chargingStartTime = 0;
bool isCharging = false;

// LED Blinking Control
unsigned long blinkInterval = 600;
bool led25Blinking = false, led50Blinking = false, led75Blinking = false, led100Blinking = false;
unsigned long lastBlinkTime25 = 0, lastBlinkTime50 = 0, lastBlinkTime75 = 0, lastBlinkTime100 = 0;

void setup() {
  pinMode(PIN_BUTTON_STOP, INPUT_PULLUP);
  pinMode(PIN_BUTTON_START, INPUT_PULLUP);
  
  for (int pin = 4; pin <= 10; ++pin)
    pinMode(pin, OUTPUT);
  
  digitalWrite(PIN_RGB_LED_GREEN, HIGH);  // Default state: Green LED ON
}

void startCharging() {
  digitalWrite(PIN_RGB_LED_GREEN, LOW);
  digitalWrite(PIN_RGB_LED_RED, HIGH);
  
  chargingStartTime = millis();
  led25Blinking = true;
  isCharging = true;
}

void stopCharging() {
  led25Blinking = led50Blinking = led75Blinking = led100Blinking = false;
  digitalWrite(PIN_LED_25, LOW);
  digitalWrite(PIN_LED_50, LOW);
  digitalWrite(PIN_LED_75, LOW);
  digitalWrite(PIN_LED_100, LOW);
  digitalWrite(PIN_RGB_LED_RED, LOW);
  digitalWrite(PIN_RGB_LED_GREEN, HIGH);
  
  isCharging = false;
}

void forceStopCharging() {
  chargingStartTime = millis() - CHARGING_TIME;  // Forcefully stop charging
}

void handleButtonPress(int buttonPin, int& buttonState, int& lastButtonState, unsigned long& debounceTime) {
  int reading = digitalRead(buttonPin);
  
  if (reading != lastButtonState) 
    debounceTime = millis();

  if ((millis() - debounceTime) > DEBOUNCE_DELAY && reading != buttonState) {
    buttonState = reading;
  }

  lastButtonState = reading;
}

void updateLEDState(int ledPin, bool& isBlinking, unsigned long& lastBlinkTime) {
  if (isBlinking && (millis() - lastBlinkTime >= blinkInterval)) {
    lastBlinkTime = millis();
    digitalWrite(ledPin, !digitalRead(ledPin));  // Toggle LED state
  }
}

void controlChargingLEDs() {
  unsigned long elapsedTime = millis() - chargingStartTime;

  if (elapsedTime > CHARGING_TIME) {
    if (elapsedTime <= CHARGING_TIME + LED_BLINK_DURATION) {
      led25Blinking = led50Blinking = led75Blinking = led100Blinking = true;
    } else {
      stopCharging();
    }
    return;
  }

  if (elapsedTime >= (CHARGING_TIME * 3 / 4)) {
    led100Blinking = true;
    led75Blinking = led50Blinking = led25Blinking = false;
  } else if (elapsedTime >= (CHARGING_TIME / 2)) {
    led75Blinking = true;
    led100Blinking = led50Blinking = led25Blinking = false;
  } else if (elapsedTime >= (CHARGING_TIME / 4)) {
    led50Blinking = true;
    led75Blinking = led100Blinking = led25Blinking = false;
  } else {
    led25Blinking = true;
    led50Blinking = led75Blinking = led100Blinking = false;
  }
}

void loop() {
  handleButtonPress(PIN_BUTTON_START, startButtonState, lastStartButtonState, lastStartDebounceTime);
  handleButtonPress(PIN_BUTTON_STOP, stopButtonState, lastStopButtonState, lastStopDebounceTime);
  
  if (startButtonState == LOW && !isCharging) {
    startCharging();
  }

  if (stopButtonState == LOW && isCharging && (millis() - stopButtonPressTime >= FORCE_STOP_TIME)) {
    forceStopCharging();
  }

  controlChargingLEDs();

  updateLEDState(PIN_LED_25, led25Blinking, lastBlinkTime25);
  updateLEDState(PIN_LED_50, led50Blinking, lastBlinkTime50);
  updateLEDState(PIN_LED_75, led75Blinking, lastBlinkTime75);
  updateLEDState(PIN_LED_100, led100Blinking, lastBlinkTime100);
}
