// #include <avr/interrupt.h>
#include <Arduino.h>

// Pin definitions
#define RGB_RED_PIN 11
#define RGB_GREEN_PIN 10
#define RGB_BLUE_PIN 9
#define START_BUTTON_PIN 2
#define DIFFICULTY_BUTTON_PIN 3

// Game state variables
// enum Difficulty { EASY, MEDIUM, HARD };
volatile bool gameActive = false;
volatile bool startButtonPressed = false;
volatile bool difficultyButtonPressed = false;
// Difficulty difficulty = EASY;
int difficulty = 1;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; // 50ms debounce delay

// Word list
const char* wordList[] = {"apple", "banana", "cherry", "date", "elderberry"};
const int wordCount = sizeof(wordList) / sizeof(wordList[0]);
int currentWordIndex = 0;
int correctWords = 0;

// Timing variables
unsigned long roundStartTime = 0;
unsigned long countdownTime = 3000;  // 3 seconds countdown
unsigned long roundDuration = 30000; // 30 seconds per round
unsigned long wordInterval = 5000;   // Default interval (changes with difficulty)
unsigned long lastWordTime = 0;

// Function declarations
void setLEDColor(int r, int g, int b);
void startButtonISR();
void difficultyButtonISR();
void startGame();
void endGame();
void cycleDifficulty();
void handleWordInput();
void displayNewWord();

String removeCharacter(const String& str, char ch) {
    String result;
    for (char c : str) {
        if (c != ch) {  // Only add characters that are not equal to the target character
            result += c;
        }
        else {
        result.remove(result.length()-1);
        
        }
    }
    return result;
}



void setup() {
  // Initialize serial communication
  Serial.begin(9600);

  // RGB LED setup
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);
  setLEDColor(255, 255, 255); // Standby (White)

  // Buttons setup
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);
  pinMode(DIFFICULTY_BUTTON_PIN, INPUT_PULLUP);

  // Attach interrupts for buttons
  attachInterrupt(digitalPinToInterrupt(START_BUTTON_PIN), startButtonISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(DIFFICULTY_BUTTON_PIN), difficultyButtonISR, FALLING);

  // Seed random generator
  randomSeed(analogRead(0));
}

void loop() {
  // Check if the game should start or stop
  if (startButtonPressed) {
    startButtonPressed = false;
    if (gameActive) {
      endGame();
    } else {
      startGame();
    }
  }

  // Handle difficulty button press
  if (difficultyButtonPressed) {
    difficultyButtonPressed = false;
    if (!gameActive) {
      cycleDifficulty();
    }
  }

  // Game logic when active
  if (gameActive) {
    if (millis() - roundStartTime >= roundDuration) {
      endGame();
    } else {
      handleWordInput();
    }
  }
}

void startGame() {
  gameActive = true;
  correctWords = 0;
  roundStartTime = millis();
  lastWordTime = millis();
  Serial.println("Game starting in:");
  for (int i = 30; i > 0; i--) {
    setLEDColor(255, 255, 0); // Yellow for countdown
    if(i%10==0)Serial.println(i/10);
    delay(50);
    setLEDColor(0, 0, 0);
    delay(50);
  }
  setLEDColor(0, 0, 255); // Green to start game
  displayNewWord();
}

void endGame() {
  gameActive = false;
  setLEDColor(255, 255, 255); // White for standby
  Serial.print("Round ended. Correct words: ");
  Serial.println(correctWords);
}

void cycleDifficulty() {
  difficulty = (difficulty + 1) % 3;
  switch (difficulty) {
    case 1: wordInterval = 5000; Serial.println("Easy mode on! - 5s: "); break;
    case 2: wordInterval = 3000; Serial.println("Medium mode on!  - 3s: "); break;
    case 3: wordInterval = 1500; Serial.println("Hard mode on! - 1.5s: "); break;
  }
}

void handleWordInput() {
  // Simulate word entry
  if (millis() - lastWordTime >= wordInterval) {
    lastWordTime = millis();
    Serial.println("New word timed out. Displaying next word.");
    displayNewWord();
  }

  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    input = removeCharacter(input, 8);
  //verifica cuvantul

    if (input.equals(wordList[currentWordIndex])) {
      correctWords++;
      setLEDColor(0, 255, 0); // Green for correct word
      displayNewWord();

      lastWordTime = 0;
    } else {
      setLEDColor(255, 0, 0); // Red for error
    }
  }
}

void displayNewWord() {
  currentWordIndex = random(0, wordCount);
  Serial.print("Type this word: ");
  Serial.println(wordList[currentWordIndex]);
}

void setLEDColor(int r, int g, int b) {
  analogWrite(RGB_RED_PIN, r);
  analogWrite(RGB_GREEN_PIN, g);
  analogWrite(RGB_BLUE_PIN, b);
}

void startButtonISR() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastDebounceTime > debounceDelay) {
    startButtonPressed = true;
    lastDebounceTime = currentMillis;
  }
}

void difficultyButtonISR() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastDebounceTime > debounceDelay) {
    difficultyButtonPressed = true;
    lastDebounceTime = currentMillis;
  }
}
