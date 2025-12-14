#include "DisplayManager.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// State
RobotMood currentMood = MOOD_IDLE;
String currentText = "Pronto"; 

// Timing management variables
unsigned long messageTimer = 0;       // When message was set
int messageDuration = 0;              // How long it should last
bool isMessageLocked = false;         // If TRUE, ignore less important updates

// Eye variables
unsigned long lastBlinkTime = 0;
int blinkInterval = 3000;
bool isBlinking = false;
int eyeX = 0; 
int eyeY = 0;

void display_setup() {
  Wire.begin(8, 9);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 fallito"));
    for(;;);
  }
  display.clearDisplay();
  display.display();
  currentMood = MOOD_IDLE;
}

// Internal function for drawing eyes
void drawEye(int x, int y, int w, int h, bool blink, int pupilX, int pupilY) {
  if (blink) {
    display.fillRect(x, y + h/2 - 2, w, 4, SSD1306_WHITE);
  } else {
    display.fillRoundRect(x, y, w, h, 5, SSD1306_WHITE);
    int centerX = x + w/2;
    int centerY = y + h/2;
    int pX = constrain(pupilX, -5, 5);
    int pY = constrain(pupilY, -5, 5);
    display.fillCircle(centerX + pX, centerY + pY, h/4, SSD1306_BLACK);
  }
}

// -------------------------------
void display_set_mood(RobotMood mood) {
  // Change mood without locking screen
  currentMood = mood;
  eyeX = 0;
  eyeY = 0;
}

void display_set_text(String text) {
  // Change text without locking screen
  currentText = text;
}

// -------------------------------

// 1. HIGH PRIORITY: Show message and LOCK screen
void display_show_temporary(String text, RobotMood mood, int duration) {
  currentText = text;
  currentMood = mood;
  messageDuration = duration;
  messageTimer = millis();
  isMessageLocked = true; // Lock screen
  
  // Reset eyes
  eyeX = 0; 
  eyeY = 0;
}

// 2. LOW PRIORITY: Update status only if screen is NOT locked
void display_update_status(String text, RobotMood mood) {
  // If there's an important message displaying, IGNORE this update
  if (isMessageLocked) {
    return; 
  }
  currentText = text;
  currentMood = mood;
}

// 3. FORCE RESET
void display_reset() {
  isMessageLocked = false;
  currentText = "Ready";
  currentMood = MOOD_IDLE;
}

// Animation loop
void display_animate() {
  unsigned long now = millis();

  // --- CONTROLLO SCADENZA TIMER ---
  if (isMessageLocked && (now - messageTimer > messageDuration)) {
    isMessageLocked = false; // Unlock
    currentMood = MOOD_IDLE; // Return to default mood
    currentText = "Ready";
  }

  display.clearDisplay();

  // 1. TEXT (Bottom)
  display.drawLine(0, 48, 128, 48, SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds(currentText, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 53);
  display.print(currentText);

  // 2. EYES (Top)
  // Blink logic
  if (currentMood == MOOD_IDLE || currentMood == MOOD_LOOKING) {
    if (!isBlinking && (now - lastBlinkTime > blinkInterval)) {
      isBlinking = true; lastBlinkTime = now;
    } else if (isBlinking && (now - lastBlinkTime > 150)) {
      isBlinking = false; lastBlinkTime = now; blinkInterval = random(2000, 6000);
    }
  } else { isBlinking = false; }

  // Draw faces
  switch (currentMood) {
    case MOOD_IDLE:
      if (!isBlinking && random(0, 100) < 5) eyeX = random(-5, 6);
      drawEye(20, 5, 30, 35, isBlinking, eyeX, 0);
      drawEye(78, 5, 30, 35, isBlinking, eyeX, 0);
      break;
    case MOOD_LOOKING:
      drawEye(20, 2, 30, 40, isBlinking, 0, 0);
      drawEye(78, 2, 30, 40, isBlinking, 0, 0);
      break;
    case MOOD_THINKING:
      eyeY = (int)(sin(now / 200.0) * 5); 
      drawEye(25, 10, 25, 25, false, 0, eyeY);
      drawEye(78, 10, 25, 25, false, 0, eyeY);
      break;
    case MOOD_HAPPY:
      display.fillCircle(35, 20, 15, SSD1306_WHITE);
      display.fillCircle(35, 15, 15, SSD1306_BLACK);
      display.fillCircle(93, 20, 15, SSD1306_WHITE);
      display.fillCircle(93, 15, 15, SSD1306_BLACK);
      break;
    case MOOD_CONFUSED:
      drawEye(20, 10, 30, 30, false, 0, 0);
      drawEye(85, 15, 15, 15, false, 0, 0);
      break;
    case MOOD_SAD:
      drawEye(20, 20, 30, 20, false, 0, 5);
      drawEye(78, 20, 30, 20, false, 0, 5);
      break;
  }
  display.display();
}