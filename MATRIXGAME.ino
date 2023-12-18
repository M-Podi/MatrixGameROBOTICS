#include <LiquidCrystal.h>
#include "LedControl.h"
#include <EEPROM.h>
void final();
void highScores();
void moveBullets();
void fireBullet(int dx, int dy);
void updateBullets();
void Menu();
void Game();
void updateMenu();
void drawMap();
void readJoystick();
void animateFallingBlocks();
void movePlayer(int dx, int dy);
void won();
void updateVisibleArea();
void settingsMenu();
void handleSettingsInput();
void handleHighScoresInput();
int readHighScoreFromEEPROM(int address);
void writeHighScoreToEEPROM(int address, int highScore);

const byte dinPin = 12;
const byte clockPin = 11;
const byte loadPin = 6;
const byte matrixSize = 8;

// Initialize the LED control
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);
const int lcdBacklightPin = 10;

// Player position
int playerX = 0;
int playerY = 0;

int visibleAreaX = 0;  // X-coordinate of the top-left corner of the visible area
int visibleAreaY = 0;  // Y-coordinate of the top-left corner of the visible area

struct GameMap {
  byte* mapData;  // Pointer to the start of the map data
  int mapSize;    // Size of the map (assuming a square map)
};

GameMap currentGameMap;


// Define the map
const int largeMapSize = 16;  // Size of the larger map
byte largeGameMap[largeMapSize][largeMapSize] = {
  { 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
  { 0, 3, 3, 3, 3, 0, 3, 1, 1, 0, 3, 3, 0, 3, 0, 1 },
  { 0, 0, 3, 3, 0, 0, 1, 0, 1, 3, 3, 1, 3, 1, 1, 1 },
  { 1, 3, 1, 1, 0, 1, 3, 3, 0, 3, 1, 1, 1, 1, 1, 1 },
  { 1, 0, 0, 1, 0, 0, 1, 1, 1, 3, 0, 1, 0, 0, 1, 1 },
  { 1, 1, 0, 1, 3, 3, 1, 0, 0, 0, 3, 3, 0, 1, 3, 1 },
  { 1, 0, 0, 1, 1, 3, 0, 1, 3, 1, 3, 3, 1, 1, 3, 1 },
  { 1, 0, 1, 0, 3, 1, 3, 1, 1, 3, 1, 1, 1, 0, 1, 1 },
  { 1, 3, 0, 1, 1, 0, 1, 1, 3, 0, 0, 0, 0, 3, 1, 1 },
  { 1, 0, 3, 3, 0, 1, 3, 0, 0, 0, 1, 3, 1, 0, 0, 1 },
  { 1, 1, 0, 1, 1, 3, 0, 3, 3, 1, 3, 3, 0, 3, 0, 1 },
  { 1, 0, 3, 3, 1, 3, 3, 3, 0, 3, 1, 1, 1, 1, 3, 1 },
  { 1, 0, 1, 1, 3, 0, 0, 0, 0, 0, 1, 0, 3, 3, 1, 0 },
  { 1, 3, 0, 1, 0, 0, 3, 1, 3, 3, 0, 0, 1, 3, 0, 0 },
  { 1, 0, 0, 3, 3, 3, 3, 3, 0, 0, 0, 0, 3, 3, 0, 0 },
  { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0 }
};

const int mediumMapSize = 10;
byte mediumGameMap[mediumMapSize][mediumMapSize] = {
  { 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
  { 0, 0, 0, 1, 0, 0, 1, 0, 3, 1 },
  { 1, 0, 0, 3, 3, 0, 0, 3, 0, 1 },
  { 1, 1, 3, 0, 1, 1, 1, 1, 3, 1 },
  { 1, 0, 0, 3, 3, 1, 3, 1, 1, 1 },
  { 1, 0, 3, 0, 1, 1, 0, 3, 1, 1 },
  { 1, 3, 3, 3, 1, 0, 1, 0, 1, 1 },
  { 1, 3, 1, 0, 3, 0, 1, 3, 3, 1 },
  { 1, 1, 0, 3, 1, 3, 0, 0, 3, 0 },
  { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0 }
};




const int smallMapSize = 8;
byte smallGameMap[smallMapSize][smallMapSize] = {
  { 0, 0, 1, 1, 1, 1, 1, 1 },
  { 0, 0, 1, 0, 0, 1, 0, 1 },
  { 1, 0, 1, 0, 1, 1, 0, 1 },
  { 1, 3, 1, 1, 0, 0, 1, 1 },
  { 1, 0, 1, 0, 0, 0, 1, 1 },
  { 1, 0, 0, 0, 0, 1, 0, 1 },
  { 1, 1, 0, 1, 0, 0, 3, 0 },
  { 1, 1, 1, 1, 1, 1, 0, 0 }
};



int currentMapSize = 0;
int winningPosX = largeMapSize - 1;
int winningPosY = largeMapSize - 1;


bool playerMoved = false;

// Blinking control variables
unsigned long previousMillis = 0;  // stores last update time
const long interval = 500;         // interval at which to blink (milliseconds)
bool playerLedState = false;       // current state of the player's LED

// Movement delay variables
const long moveInterval = 200;   // Time in milliseconds between moves
unsigned long lastMoveTime = 0;  // Last time the player moved

unsigned long previousWallMillis = 0;
const long wallInterval = 1;
bool wallLedState = false;

// LCD pins
const int rs = A2, en = 8, d4 = 7, d5 = 3, d6 = 5, d7 = 4;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Joystick pins
const int joyX = A1;
const int joyY = A0;
const int joySW = 2;

bool buttonPressHandled = false;

unsigned long lastJoystickUpdateTime = 0;
const long joystickUpdateInterval = 200;

unsigned long lastButtonPressTime = 0;  // Declare this globally
const long debounceDelay = 200;         // Debounce delay in milliseconds



int menuItem = 0;
const int menuSize = 4;
const char* menuOptions[menuSize] = { "Start", "Highest Scores", "Settings", "About" };

int topItem = 0;

const long bulletMoveInterval = 100;  // Bullet movement interval in milliseconds

int settingsMenuItem = 0;

int highScoresMenuItem = 0;
const int highScoresMenuSize = 4;
const char* highScoresMenuOptions[highScoresMenuSize] = { "Easy", "Medium", "Hard", "Back" };


struct Bullet {
  int x, y;
  int dx, dy;                      // Direction of the bullet
  bool active;                     // Is the bullet currently active?
  bool isOn;                       // Track the LED state for blinking
  unsigned long lastMoveTime = 0;  // Last time the bullet moved
};

int elapsedTime = 0;


byte getValueAt(int x, int y) {
  if (x >= 0 && x < currentGameMap.mapSize && y >= 0 && y < currentGameMap.mapSize) {
    return currentGameMap.mapData[y * currentGameMap.mapSize + x];
  }
  return 0;  // Return a default value if out of bounds
}

const int maxBullets = 5;  // Maximum number of bullets at a time
Bullet bullets[maxBullets];

int lastPlayerDX = 0;  // Last horizontal movement direction
int lastPlayerDY = 0;  // Last vertical movement direction


unsigned long bulletPreviousMillis = 0;
const long bulletInterval = 100;  // Faster blink rate for bullets


int currentHighScore;

int highScoreAddress = 0;  // Starting address in EEPROM for high score

const int lcdBrightnessAddress = 30;     // EEPROM address for LCD brightness
const int matrixBrightnessAddress = 60;  // EEPROM address for Matrix brightness
const int difficultyAddress = 90;        // EEPROM address for difficulty
const int songIndexAddress = 125;  // EEPROM address for song index


int lcdBrightness = 90;    // Default LCD brightness
int matrixBrightness = 8;  // Default Matrix brightness (0-15 for LedControl)

enum Setting { SETTING_LCD_BRIGHTNESS,
               SETTING_MATRIX_BRIGHTNESS,
               SETTING_DIFFICULTY,
               SETTING_MUSIC,
               SETTING_CHANGE_NAME,
               BACK };


// Current setting being adjusted
Setting currentSetting = SETTING_LCD_BRIGHTNESS;

const int NUM_SETTINGS = 3;


const int topScoresCount = 3;  // Number of top scores to store

int easyHighScores[topScoresCount] = { INT8_MAX, INT8_MAX, INT8_MAX };
int mediumHighScores[topScoresCount] = { INT8_MAX, INT8_MAX, INT8_MAX };
int hardHighScores[topScoresCount] = { INT8_MAX, INT8_MAX, INT8_MAX };

const int easyHighScoreAddress = 100;    // Starting EEPROM address for easy scores
const int mediumHighScoreAddress = 106;
const int hardHighScoreAddress = 112;
const int playerNameAddress = 120;




void writeHighScoreToEEPROM(int address, int highScore) {
  EEPROM.put(address, highScore);
}


void adjustLCDBrightness(int change) {
  lcdBrightness += change * 5;
  lcdBrightness = constrain(lcdBrightness, 0, 255);
  analogWrite(lcdBacklightPin, lcdBrightness);
}

void turnOffMatrix() {
  for (int row = 0; row < matrixSize; ++row) {
    for (int col = 0; col < matrixSize; ++col) {
      lc.setLed(0, row, col, false);
    }
  }
}


void adjustMatrixBrightness(int change) {
  matrixBrightness += change;
  matrixBrightness = constrain(matrixBrightness, 0, 15);
  lc.setIntensity(0, matrixBrightness);

  // Light up the matrix to show new brightness level
  for (int row = 0; row < matrixSize; ++row) {
    for (int col = 0; col < matrixSize; ++col) {
      lc.setLed(0, row, col, true);
    }
  }
}



void moveBullets() {
  unsigned long currentMillis = millis();
  for (int i = 0; i < maxBullets; i++) {
    if (bullets[i].active && currentMillis - bullets[i].lastMoveTime > bulletMoveInterval) {
      // Update the last move time

      bullets[i].lastMoveTime = currentMillis;

      // Proposed new position for the bullet
      int newX = bullets[i].x + bullets[i].dx;
      int newY = bullets[i].y + bullets[i].dy;
      byte mapValue = getValueAt(newX, newY);


      // Check bounds and wall collision
      if (newX < 0 || newX >= currentGameMap.mapSize || newY < 0 || newY >= currentGameMap.mapSize) {
        bullets[i].active = false;
      } else if (mapValue == 1 || mapValue == 2) {
        bullets[i].active = false;
      } else if (mapValue == 3) {
        currentGameMap.mapData[newY * currentGameMap.mapSize + newX] = 0;
        bullets[i].active = false;
      } else {
        bullets[i].x = newX;
        bullets[i].y = newY;
      }
    }
  }
}




void fireBullet(int dx, int dy) {
  if (dx == 0 && dy == 0) return;  // No bullet fired if player hasn't moved

  for (int i = 0; i < maxBullets; i++) {
    if (!bullets[i].active) {
      bullets[i].x = playerX;
      bullets[i].y = playerY;
      bullets[i].dx = dx;
      bullets[i].dy = dy;
      bullets[i].active = true;
      bullets[i].lastMoveTime = millis();  // Initialize the last move time
      break;
    }
  }
}


void updateBullets() {
  unsigned long currentMillis = millis();
  if (currentMillis - bulletPreviousMillis >= bulletInterval) {
    bulletPreviousMillis = currentMillis;
    for (int i = 0; i < maxBullets; i++) {
      if (bullets[i].active) {
        bullets[i].isOn = !bullets[i].isOn;  // Toggle the bullet's state
        lc.setLed(0, bullets[i].y, bullets[i].x, bullets[i].isOn);
      }
    }
  }
}


const int buzzerPin = 13;
const int buzzerPin2 = 9;


enum State { MENU,
             HIGHSCORES,
             HIGHSCORES_EASY,
             HIGHSCORES_MEDIUM,
             HIGHSCORES_HARD,
             SETTINGS,
             GAME,
             STARTING,
             WON,
             FINAL,
             SETTINGS_DIFFICULTY,
             INTRO,
             ABOUT };

int difficulty = 0;  // Default difficulty level
const char* difficulties[3] = { "Easy", "Medium", "Hard" };

int songIndex = 3;
const char* songs[3] = { "Off", "Song1", "Song2" };

unsigned long introStartTime;

State currentState;

String aboutText = "This is a maze navigation game, utilizing an 8x8 LED matrix. Players are represented by a blinking LED, navigating through a maze of solid and breakable walls. The goal is to find the shortest route to the maze's end.";
int aboutTextPosition = 0;  // Current scroll position in the about text

int selectedChars[5] = { 65, 65, 65, 65, 65 };  // ASCII values for 'A'
int activeCharIndex = 0;                        // Index of the currently active letter



// Define the tempos (beats per minute)
const int tempo1 = 126;  // Tempo for song 1
const int tempo2 = 144;  // Tempo for song 2

// Melody 1: Frequencies stored in PROGMEM
const int melody1[] PROGMEM = { 659, 622, 659, 622, 659, 493, 587, 523, 440, 261, 329, 440, 493, 329, 415, 493, 523, 329, 659, 622, 659, 622, 659, 493, 587, 523, 440, 261, 329, 440, 493, 261, 523, 493, 440 };
const int melodyLength1 = sizeof(melody1) / sizeof(melody1[0]);

// Melody 2: Frequencies stored in PROGMEM
const int melody2[] PROGMEM = { 369, 523, 622, 622, 369, 369, 523, 622, 622, 369, 587, 698, 698, 466, 587, 698, 932, 830, 783, 622, 523, 523, 783, 783, 622, 523, 523, 587, 466, 391, 391, 587, 587, 466, 391, 391 };
const int melodyLength2 = sizeof(melody2) / sizeof(melody2[0]);

// Variables to control the melody playback
unsigned int noteIndex = 0;
unsigned long noteDuration = 0;
unsigned long lastNoteChangeTime = 0;
bool playingNote = false;
unsigned long noteStopTime = 0;
const unsigned long noteGap = 5;  // gap between notes in milliseconds



void playSong() {
  unsigned long currentMillis = millis();

  if (songIndex == 1 && noteIndex < melodyLength1) {
    playMelody(melody1, melodyLength1, tempo1, currentMillis);
  } else if (songIndex == 2 && noteIndex < melodyLength2) {
    playMelody(melody2, melodyLength2, tempo2, currentMillis);
  } else if (songIndex == 3) {
    noTone(buzzerPin2);
  } else {
    noteIndex = 0;
    playingNote = false;
    noteStopTime = currentMillis;
  }
}

void playMelody(const int melody[], int melodyLength, int tempo, unsigned long currentMillis) {
  if (playingNote && currentMillis - lastNoteChangeTime >= noteDuration) {
    noTone(buzzerPin2);
    playingNote = false;
    noteStopTime = currentMillis;
  } else if (!playingNote && currentMillis - noteStopTime >= noteGap) {
    noteDuration = (60.0 / tempo * 1000) * 4 / 8;
    int frequency = pgm_read_word_near(melody + noteIndex);
    tone(buzzerPin2, frequency, noteDuration);

    playingNote = true;
    lastNoteChangeTime = currentMillis;

    noteIndex++;
  }
}


void selectGameMapBasedOnDifficulty() {
  switch (difficulty) {
    case 0:  // Easy
      currentGameMap.mapData = &smallGameMap[0][0];
      currentGameMap.mapSize = smallMapSize;
      break;
    case 1:  // Medium
      currentGameMap.mapData = &mediumGameMap[0][0];
      currentGameMap.mapSize = mediumMapSize;
      break;
    case 2:  // Hard
      currentGameMap.mapData = &largeGameMap[0][0];
      currentGameMap.mapSize = largeMapSize;
      break;
  }

  // Reset player position and winning position for the new map
  playerX = 0;
  playerY = 0;
  winningPosX = currentGameMap.mapSize - 1;
  winningPosY = currentGameMap.mapSize - 1;
}

void intro() {
  unsigned long currentMillis = millis();
  if (currentMillis - introStartTime < 5000) {
    // Display the intro message for the first 5 seconds
    lcd.setCursor(0, 0);
    lcd.print("Welcome to");
    lcd.setCursor(0, 1);
    lcd.print("MAZE RUNNER");
  } else {
    // After 5 seconds, switch to the menu state
    currentState = MENU;
    updateMenu();
  }
}

void setup() {
  Serial.begin(9600);

  EEPROM.get(lcdBrightnessAddress, lcdBrightness);
  EEPROM.get(matrixBrightnessAddress, matrixBrightness);
  EEPROM.get(difficultyAddress, difficulty);
  EEPROM.get(songIndexAddress, songIndex);

  // Constrain values to valid ranges
  lcdBrightness = constrain(lcdBrightness, 0, 255);
  matrixBrightness = constrain(matrixBrightness, 0, 15);
  difficulty = constrain(difficulty, 0, 2);
  songIndex = constrain(songIndex, 0, 2);


  analogWrite(lcdBacklightPin, lcdBrightness);
  lc.setIntensity(0, matrixBrightness);

  for (int i = 0; i < 5; i++) {
    selectedChars[i] = EEPROM.read(playerNameAddress + i);
    if (selectedChars[i] < 65 || selectedChars[i] > 90) {
      selectedChars[i] = 65;  // Default to 'A' if the stored value is invalid
    }
  }

  for (int i = 0; i < topScoresCount; i++) {
    easyHighScores[i] = readHighScoreFromEEPROM(easyHighScoreAddress + i * sizeof(int));
    mediumHighScores[i] = readHighScoreFromEEPROM(mediumHighScoreAddress + i * sizeof(int));
    hardHighScores[i] = readHighScoreFromEEPROM(hardHighScoreAddress + i * sizeof(int));
  }

  currentState = INTRO;  // Starting state
  introStartTime = millis();
  pinMode(lcdBacklightPin, OUTPUT);

  analogWrite(lcdBacklightPin, lcdBrightness);
  lc.setIntensity(0, matrixBrightness);

  lcd.begin(16, 2);
  pinMode(joyX, INPUT);
  pinMode(joyY, INPUT);
  pinMode(joySW, INPUT_PULLUP);  // Enable internal pull-up
  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);
  pinMode(buzzerPin, OUTPUT);
  // Display initial menu
  largeGameMap[playerY][playerX] = 2;
  mediumGameMap[playerY][playerX] = 2;
  smallGameMap[playerY][playerX] = 2;

  currentGameMap.mapData = &smallGameMap[0][0];
  currentGameMap.mapSize = smallMapSize;

  winningPosX = currentGameMap.mapSize - 1;
  winningPosY = currentGameMap.mapSize - 1;
}




void loop() {
  playSong();
  switch (currentState) {
    case INTRO:
      intro();
      break;
    case MENU:
      Menu();
      break;
    case HIGHSCORES:
      handleHighScoresInput();
      break;
    case SETTINGS_DIFFICULTY:
      handleSettingsInput();
      break;
    case GAME:
      Game();
      break;
    case STARTING:
      animateFallingBlocks();
      break;
    case WON:
      won();
      break;
    case FINAL:
      final();
      break;
    case ABOUT:
      aboutSection();
      break;
  }
}

bool hasUpdatedHighScore = false;

void final() {
  int* currentScores;
  int scoreAddress;

  switch (difficulty) {
    case 0:
      currentScores = easyHighScores;
      scoreAddress = easyHighScoreAddress;
      break;
    case 1:
      currentScores = mediumHighScores;
      scoreAddress = mediumHighScoreAddress;
      break;
    case 2:
      currentScores = hardHighScores;
      scoreAddress = hardHighScoreAddress;
      break;
  }

  if (!hasUpdatedHighScore) {
    // Find where to insert the new score
    int insertAt = -1;
    for (int i = 0; i < topScoresCount; i++) {
      if (elapsedTime < currentScores[i]) {
        insertAt = i;
        break;
      }
    }

    // Insert new score and shift down others
    if (insertAt != -1) {
      for (int i = topScoresCount - 1; i > insertAt; i--) {
        currentScores[i] = currentScores[i - 1];
      }
      currentScores[insertAt] = elapsedTime;
    }

    // Write updated scores to EEPROM and display them
    for (int i = 0; i < topScoresCount; i++) {
      writeHighScoreToEEPROM(scoreAddress + i * sizeof(int), currentScores[i]);
      Serial.print("Updated Score ");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.println(currentScores[i]);
    }
    hasUpdatedHighScore = true;
  }

  // Display the score on the LCD

  lcd.setCursor(0, 0);
  lcd.print("Score obtained:");
  lcd.setCursor(0, 1);
  lcd.print(elapsedTime);
  unsigned long currentMillis = millis();
  if (currentMillis - lastJoystickUpdateTime > joystickUpdateInterval) {
    int yValue = analogRead(joyY);
    bool buttonPressed = !digitalRead(joySW);

    // Check if joystick button is pressed to return to the main menu
    if (buttonPressed) {
      tone(buzzerPin, 1000, 200);
      hasUpdatedHighScore = false;
      currentState = MENU;
      updateMenu();
    }

    lastJoystickUpdateTime = currentMillis;
  }
}


void aboutSection() {
  lcd.setCursor(0, 0);
  lcd.print("About Section:");
  lcd.setCursor(0, 1);
  lcd.print(aboutText.substring(aboutTextPosition, aboutTextPosition + 16));
  unsigned long currentMillis = millis();
  if (currentMillis - lastJoystickUpdateTime > joystickUpdateInterval) {
    int yValue = analogRead(joyY);
    bool buttonPressed = !digitalRead(joySW);

    // Check if joystick button is pressed to return to the main menu
    if (buttonPressed) {
      currentState = MENU;
      updateMenu();
    }

    // Scroll up
    if (yValue < 300 && aboutTextPosition > 0) {
      aboutTextPosition--;
    }
    // Scroll down
    else if (yValue > 700 && aboutTextPosition < aboutText.length() - 16) {
      aboutTextPosition++;
    }
    if (buttonPressed && (currentMillis - lastButtonPressTime > debounceDelay)) {
      lastButtonPressTime = currentMillis;  // Update last button press time
      tone(buzzerPin, 1000, 200);
      currentState = MENU;
      updateMenu();
    }
    lastJoystickUpdateTime = currentMillis;
  }
}


void Menu() {
  unsigned long currentMillis = millis();

  // Check if the joystick debounce interval has passed
  if (currentMillis - lastJoystickUpdateTime > joystickUpdateInterval) {
    int yValue = analogRead(joyY);
    bool buttonPressed = !digitalRead(joySW);  // Inverted logic due to pull-up

    // Joystick navigation: Up
    if (yValue < 300) {
      menuItem--;
      if (menuItem < 0) menuItem = 0;              // Prevent going above the first item
      if (menuItem < topItem) topItem = menuItem;  // Scroll up
      updateMenu();
      lastJoystickUpdateTime = currentMillis;  // Update the last joystick update time
    }
    // Joystick navigation: Down
    else if (yValue > 700) {
      menuItem++;
      if (menuItem >= menuSize) menuItem = menuSize - 1;   // Prevent going below the last item
      if (menuItem > topItem + 1) topItem = menuItem - 1;  // Scroll down
      updateMenu();
      lastJoystickUpdateTime = currentMillis;  // Update the last joystick update time
    }

    // Select menu item
    if (buttonPressed) {
      tone(buzzerPin, 1000, 200);
      switch (menuItem) {
        case 0:
          currentState = STARTING;  // Start the game
          selectGameMapBasedOnDifficulty();
          break;
        case 1:
          currentState = HIGHSCORES;  // Switch to highscore state
          highScoresMenuItem = 0;
          highScoresMenu();
          break;
        case 2:
          currentState = SETTINGS_DIFFICULTY;
          settingsMenuItem = 0;  // Reset settings menu item to the first option
          settingsMenu();
          break;
        case 3:
          currentState = ABOUT;
          aboutTextPosition = 0;  // Reset scroll position
          break;
      }
    }
    lastJoystickUpdateTime = currentMillis;  // Update the last joystick update time
  }
}


unsigned long gameStartTime;

void Game() {
  unsigned long currentMillis = millis();
  elapsedTime = (currentMillis - gameStartTime) / 1000;  // Convert to seconds

  String playerName = "";
  for (int i = 0; i < 5; i++) {
    playerName += char(selectedChars[i]);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(playerName + "'s time:");
  lcd.setCursor(0, 1);
  lcd.print(elapsedTime);

  // Update blinking only when the player is stationary
  if (!playerMoved && currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    playerLedState = !playerLedState;  // Toggle the player's LED state
  } else if (playerMoved) {
    playerLedState = true;  // Keep the player LED on when moving
    playerMoved = false;    // Reset the movement flag
  }

  if (currentMillis - previousWallMillis >= wallInterval) {
    previousWallMillis = currentMillis;
    wallLedState = !wallLedState;
  }

  readJoystick();
  moveBullets();  // Move bullets each game loop iteration
  updateBullets();
  drawMap();
}

void updateMenu() {
  lcd.clear();
  for (int i = 0; i < 2; i++) {
    int itemIndex = topItem + i;
    if (itemIndex < menuSize) {
      lcd.setCursor(0, i);
      if (itemIndex == menuItem) {
        lcd.print(">");
      } else {
        lcd.print(" ");
      }
      lcd.print(menuOptions[itemIndex]);
    }
  }
}

void drawMap() {
  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      int mapRow = visibleAreaY + row;
      int mapCol = visibleAreaX + col;
      if (mapRow < currentGameMap.mapSize && mapCol < currentGameMap.mapSize) {
        byte mapValue = getValueAt(mapCol, mapRow);

        if (mapValue == 1) {
          lc.setLed(0, row, col, true);  // Wall
        } else if (mapValue == 2) {
          lc.setLed(0, row, col, playerLedState);  // Player, blinking
        } else if (mapValue == 3) {
          lc.setLed(0, row, col, wallLedState);  // Breakable walls blinking
        } else {
          lc.setLed(0, row, col, false);  // Empty space
        }
      }
    }
  }
  for (int i = 0; i < maxBullets; i++) {
    if (bullets[i].active) {
      int visibleBulletX = bullets[i].x - visibleAreaX;
      int visibleBulletY = bullets[i].y - visibleAreaY;
      if (visibleBulletX >= 0 && visibleBulletX < matrixSize && visibleBulletY >= 0 && visibleBulletY < matrixSize) {
        lc.setLed(0, visibleBulletY, visibleBulletX, bullets[i].isOn);
      }
    }
  }
}




void readJoystick() {
  unsigned long currentMillis = millis();
  int xVal = analogRead(joyX);
  int yVal = analogRead(joyY);
  bool buttonPressed = digitalRead(joySW) == LOW;

  // Deadzone threshold
  int deadzone = 300;

  bool joystickMoved = false;  // Flag to check if joystick has moved

  if (xVal < (512 - deadzone) || xVal > (512 + deadzone)) {
    joystickMoved = true;
  }

  if (yVal < (512 - deadzone) || yVal > (512 + deadzone)) {
    joystickMoved = true;
  }

  if (joystickMoved && currentMillis - lastMoveTime > moveInterval) {
    // Reset the direction
    lastPlayerDX = 0;
    lastPlayerDY = 0;

    if (xVal < (512 - deadzone)) {
      movePlayer(-1, 0);
      lastPlayerDX = -1;
    } else if (xVal > (512 + deadzone)) {
      movePlayer(1, 0);
      lastPlayerDX = 1;
    }

    if (yVal > (512 + deadzone)) {
      movePlayer(0, -1);
      lastPlayerDY = -1;
    } else if (yVal < (512 - deadzone)) {
      movePlayer(0, 1);
      lastPlayerDY = 1;
    }

    lastMoveTime = currentMillis;
    playerMoved = true;
  }


  static bool lastButtonState = HIGH;  // Track the last state

  if (buttonPressed && lastButtonState != buttonPressed) {
    fireBullet(lastPlayerDX, lastPlayerDY);
    tone(buzzerPin, 1000, 200);
  }
  lastButtonState = buttonPressed;
  // Fire bullet on button press
  if (buttonPressed && lastButtonState != buttonPressed) {
    // Call fireBullet() with the last known player direction
    fireBullet(lastPlayerDX, lastPlayerDY);
  }

  lastButtonState = buttonPressed;  // Update last button state
}

// Global variables for animateFallingBlocks function
int fallingBlockRow = matrixSize - 1;
int fallingBlockCol = 0;
int fallingBlockCurrentRow = 0; // Tracks the current falling position of the block
unsigned long lastBlockFallTime = 0;
const long blockFallInterval = 15; // Interval at which each block falls

void animateFallingBlocks() {
  unsigned long currentMillis = millis();

  if (fallingBlockRow >= 0) {
    if (currentMillis - lastBlockFallTime > blockFallInterval) {
      lastBlockFallTime = currentMillis;

      // Turn off the previous position of the falling block
      if (fallingBlockCurrentRow > 0) {
        lc.setLed(0, fallingBlockCurrentRow - 1, fallingBlockCol, false);
      }

      // Animate the current block
      if (getValueAt(fallingBlockCol, fallingBlockRow) == 1) {
        lc.setLed(0, fallingBlockCurrentRow, fallingBlockCol, true);
      }

      fallingBlockCurrentRow++;

      // Check if the block has reached its final position
      if (fallingBlockCurrentRow > fallingBlockRow) {
        fallingBlockCol++;
        fallingBlockCurrentRow = 0; // Reset for the next block

        // Move to the next column or row
        if (fallingBlockCol >= matrixSize) {
          fallingBlockCol = 0;
          fallingBlockRow--;
        }
      }
    }
  } else {
    // Animation complete, change state
    gameStartTime = millis();
    currentState = GAME;
    // Reset variables for the next time the animation is played
    fallingBlockRow = matrixSize - 1;
    fallingBlockCol = 0;
    fallingBlockCurrentRow = 0;
  }
}




void movePlayer(int dx, int dy) {
  int newX = playerX + dx;
  int newY = playerY + dy;

  if (newX >= 0 && newX < currentGameMap.mapSize && newY >= 0 && newY < currentGameMap.mapSize && getValueAt(newX, newY) == 0) {
    currentGameMap.mapData[playerY * currentGameMap.mapSize + playerX] = 0;  // Clear old position
    playerX = newX;
    playerY = newY;
    currentGameMap.mapData[playerY * currentGameMap.mapSize + playerX] = 2;  // Set new position
    updateVisibleArea();

    if (playerX == winningPosX && playerY == winningPosY) {
      currentState = WON;
    }
  }

  if (newX != playerX || newY != playerY) {
    playerMoved = true;
  }
  if (playerMoved) {
    lastPlayerDX = dx;
    lastPlayerDY = dy;
    updateVisibleArea();
  }
}

void won() {
  const int numberOfFireworks = 40;  // Total number of fireworks to display
  const int displayTime = 100;       // Time each firework is displayed in milliseconds
  lc.clearDisplay(0);
  for (int i = 0; i < numberOfFireworks; i++) {
    int x = random(matrixSize);
    int y = random(matrixSize);

    // Light up a random LED
    lc.setLed(0, x, y, true);
    delay(displayTime);

    // Turn off the LED
    lc.setLed(0, x, y, false);

    delay(50);
  }
  currentState = FINAL;
  lcd.clear();
}


void updateVisibleArea() {
  // Center the visible area on the player as much as possible
  visibleAreaX = max(0, min(playerX - matrixSize / 2, currentGameMap.mapSize - matrixSize));
  visibleAreaY = max(0, min(playerY - matrixSize / 2, currentGameMap.mapSize - matrixSize));

  // Additional checks for map boundaries
  if (playerX < matrixSize / 2) {
    visibleAreaX = 0;
  } else if (playerX > currentGameMap.mapSize - matrixSize / 2) {
    visibleAreaX = currentGameMap.mapSize - matrixSize;
  }

  if (playerY < matrixSize / 2) {
    visibleAreaY = 0;
  } else if (playerY > currentGameMap.mapSize - matrixSize / 2) {
    visibleAreaY = currentGameMap.mapSize - matrixSize;
  }
}


void handleNameChange(int xValue, int yValue) {
  // Adjust the character (cycle through letters) using xValue
  if (abs(xValue - 512) > 300) {
    int changeX = (xValue < 512) ? -1 : 1;
    selectedChars[activeCharIndex] += changeX;

    // Wrap around logic for the character
    if (selectedChars[activeCharIndex] < 65) selectedChars[activeCharIndex] = 90;
    if (selectedChars[activeCharIndex] > 90) selectedChars[activeCharIndex] = 65;
  }

  // Move to the next/previous character position using yValue
  if (abs(yValue - 512) > 300) {
    int changeY = (yValue < 512) ? -1 : 1;
    activeCharIndex += changeY;

    // Wrap around logic for activeCharIndex
    if (activeCharIndex < 0) activeCharIndex = 4;
    if (activeCharIndex > 4) activeCharIndex = 0;
  }
}



// Additional global variable to track the mode
bool inAdjustmentMode = false;

const int settingsMenuSize = 6;
const char* settingsMenuOptions[settingsMenuSize] = { "LCD Brightness", "Game Brightness", "Difficulty", "Music", "Player Name", "Back" };

void settingsMenu() {
  lcd.clear();

  if (!inAdjustmentMode) {
    // Display settings menu options
    turnOffMatrix();
    for (int i = 0; i < 2; i++) {
      int itemIndex = settingsMenuItem + i;
      if (itemIndex < settingsMenuSize) {
        lcd.setCursor(0, i);
        if (itemIndex == settingsMenuItem) {
          lcd.print(">");
        } else {
          lcd.print(" ");
        }
        lcd.print(settingsMenuOptions[itemIndex]);
      }
    }
  } else {
    // Display the name of the setting being adjusted on the first row
    lcd.setCursor(0, 0);
    switch (currentSetting) {
      case SETTING_LCD_BRIGHTNESS:
        lcd.print("LCD Brightness:");
        break;
      case SETTING_MATRIX_BRIGHTNESS:
        lcd.print("Mat Brightness:");
        break;
      case SETTING_DIFFICULTY:
        lcd.print("Difficulty:");
        break;
      case SETTING_MUSIC:
        lcd.print("Music:");
        break;
      case SETTING_CHANGE_NAME:
        for (int i = 0; i < 5; i++) {
          lcd.setCursor(i, 0);
          lcd.print((char)selectedChars[i]);
        }
    }

    // Display the current value of the setting on the second row
    lcd.setCursor(0, 1);
    switch (currentSetting) {
      case SETTING_LCD_BRIGHTNESS:
        lcd.print(lcdBrightness);
        break;
      case SETTING_MATRIX_BRIGHTNESS:
        adjustMatrixBrightness(0);
        lcd.print(matrixBrightness);
        break;
      case SETTING_DIFFICULTY:
        lcd.print(difficulties[difficulty]);
        break;
      case SETTING_MUSIC:
        lcd.print(songs[songIndex]);
        break;
      case SETTING_CHANGE_NAME:
        lcd.setCursor(activeCharIndex, 1);  // Move to the active character's position
        lcd.print('^');                     // Print the arrow symbol
    }
  }
}




// Global variable to store the last joystick Y-value
int lastJoyYValue = 512;
bool lastButtonState = HIGH;

void handleSettingsInput() {
  unsigned long currentMillis = millis();

  // Read the joystick and button states
  int yValue = analogRead(joyY);
  int xValue = analogRead(joyX);
  bool buttonPressed = !digitalRead(joySW);

  // Check if the button state has changed
  if (buttonPressed != lastButtonState) {
    lastButtonState = buttonPressed;

    // Debounce logic for button press
    if (buttonPressed && (currentMillis - lastButtonPressTime > debounceDelay)) {
      lastButtonPressTime = currentMillis;

      if (!inAdjustmentMode) {
        if (settingsMenuItem != settingsMenuSize - 1) {  // 'Back' is selected
          inAdjustmentMode = true;
          currentSetting = static_cast<Setting>(settingsMenuItem);
        }else{
          // Save settings to EEPROM
          EEPROM.put(lcdBrightnessAddress, lcdBrightness);
          EEPROM.put(matrixBrightnessAddress, matrixBrightness);
          EEPROM.put(difficultyAddress, difficulty);
          EEPROM.put(songIndexAddress, songIndex);
          for (int i = 0; i < 5; i++) {
            EEPROM.write(playerNameAddress + i, selectedChars[i]);
          }
          selectGameMapBasedOnDifficulty();
          currentState = MENU;
          updateMenu();
          delay(300);
          return;
        }
      } else {
        // Exit adjustment mode
        inAdjustmentMode = false;
      }
      settingsMenu();  // Refresh the settings menu display
    }
  }

  // Handle joystick movement
  if (currentMillis - lastJoystickUpdateTime > joystickUpdateInterval) {
    lastJoystickUpdateTime = currentMillis;

    // Adjust settings if in adjustment mode
    if (inAdjustmentMode) {
      if (abs(yValue - 512) > 300 || abs(xValue - 512) > 300) {  // Check for significant joystick movement
        int change = (yValue > 700) ? 1 : -1;
        switch (currentSetting) {
          case SETTING_LCD_BRIGHTNESS:
            adjustLCDBrightness(change);
            break;
          case SETTING_MATRIX_BRIGHTNESS:
            adjustMatrixBrightness(change);
            break;
          case SETTING_DIFFICULTY:
            difficulty = constrain(difficulty + change, 0, 2);
            break;
          case SETTING_MUSIC:
            songIndex = constrain(songIndex + change, 0, 2);
            break;
          case SETTING_CHANGE_NAME:
            handleNameChange(xValue, yValue);
            break;
        }
        settingsMenu();  // Refresh the settings menu display
      }
    } else {
      // Navigate through settings options
      if ((yValue < 300 && settingsMenuItem > 0) || (yValue > 700 && settingsMenuItem < settingsMenuSize - 1)) {
        settingsMenuItem += (yValue < 300) ? -1 : 1;
        settingsMenu();  // Refresh the settings menu display
      }
    }
  }
}


// Global flag to indicate if we are viewing top scores
bool inViewingMode = false;
int scoreDisplayIndex = 0;  // Global variable to track the score index

void handleHighScoresInput() {
  unsigned long currentMillis = millis();
  int yValue = analogRead(joyY);
  bool buttonPressed = !digitalRead(joySW);

  if (buttonPressed && (currentMillis - lastButtonPressTime > debounceDelay)) {
    lastButtonPressTime = currentMillis;

    if (inViewingMode) {
      // Exit viewing mode
      inViewingMode = false;
      highScoresMenu();  // Refresh the high scores menu display
      return;
    } else {
      // Handle selection within the high scores menu
      switch (highScoresMenuItem) {
        case 0:
        case 1:
        case 2:
          displayTop3Scores(highScoresMenuItem);  // Display scores for the selected difficulty
          inViewingMode = true;
          scoreDisplayIndex = 0;  // Reset score display index whenever a new difficulty is selected
          break;
        case 3:
          currentState = MENU;  // Go back to the main menu
          updateMenu();
          break;
      }
    }
  }

  lastButtonState = buttonPressed;

  // Handle Joystick Movement for viewing mode
  if (inViewingMode && currentMillis - lastJoystickUpdateTime > joystickUpdateInterval) {
    lastJoystickUpdateTime = currentMillis;

    // Navigate within the high scores
    if (yValue < 300) {
      scoreDisplayIndex--;
      scoreDisplayIndex = max(scoreDisplayIndex, 0);
    } else if (yValue > 700) {
      scoreDisplayIndex++;
      scoreDisplayIndex = min(scoreDisplayIndex, topScoresCount - 2);
    }

    displayTop3Scores(highScoresMenuItem);  // Refresh high score display
  }

  // Handle Joystick Movement for menu navigation
  if (!inViewingMode && currentMillis - lastJoystickUpdateTime > joystickUpdateInterval) {
    lastJoystickUpdateTime = currentMillis;

    // Navigate within the high scores menu
    if (yValue < 300 && highScoresMenuItem > 0) {
      highScoresMenuItem--;
      highScoresMenu();
    } else if (yValue > 700 && highScoresMenuItem < highScoresMenuSize - 1) {
      highScoresMenuItem++;
      highScoresMenu();
    }
  }
}












void highScoresMenu() {
  lcd.clear();
  for (int i = 0; i < 2; i++) {
    int itemIndex = highScoresMenuItem + i;
    if (itemIndex < highScoresMenuSize) {
      lcd.setCursor(0, i);
      lcd.print((itemIndex == highScoresMenuItem) ? ">" : " ");
      lcd.print(highScoresMenuOptions[itemIndex]);
    }
  }
}


void displayTop3Scores(int difficulty) {
  int* currentScores;
  switch (difficulty) {
    case 0:
      currentScores = easyHighScores;
      break;
    case 1:
      currentScores = mediumHighScores;
      break;
    case 2:
      currentScores = hardHighScores;
      break;
  }

  // Display on LCD
  lcd.clear();
  lcd.print("Top Scores:");

  // Display scores based on the scoreDisplayIndex
  for (int i = 0; i < 2; i++) {
    int scoreIndex = scoreDisplayIndex + i;
    if (scoreIndex < topScoresCount) {
      lcd.setCursor(0, i + 1);
      lcd.print(scoreIndex);
      lcd.print(": ");
      lcd.print(currentScores[scoreIndex]);
    }
  }
}




int readHighScoreFromEEPROM(int address) {
  int highScore = EEPROM.read(address);
  return highScore;
}