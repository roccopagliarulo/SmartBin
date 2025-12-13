#include "DisplayManager.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Stato
RobotMood currentMood = MOOD_IDLE;
String currentText = "Pronto"; 

// --- VARIABILI PER GESTIONE TEMPORALE ---
unsigned long messageTimer = 0;       // Quando è stato impostato il messaggio
int messageDuration = 0;              // Quanto deve durare
bool isMessageLocked = false;         // Se TRUE, ignora aggiornamenti meno importanti

// Variabili occhi
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

// Funzione interna per gli occhi
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

// --- FUNZIONI MANCANTI (FIX) ---

void display_set_mood(RobotMood mood) {
  // Cambia l'umore senza bloccare lo schermo
  currentMood = mood;
  eyeX = 0;
  eyeY = 0;
}

void display_set_text(String text) {
  // Cambia il testo senza bloccare lo schermo
  currentText = text;
}

// -------------------------------

// 1. ALTA PRIORITÀ: Mostra un messaggio e BLOCCA lo schermo
void display_show_temporary(String text, RobotMood mood, int duration) {
  currentText = text;
  currentMood = mood;
  messageDuration = duration;
  messageTimer = millis();
  isMessageLocked = true; // BLOCCA lo schermo
  
  // Reset occhi
  eyeX = 0; 
  eyeY = 0;
}

// 2. BASSA PRIORITÀ: Aggiorna lo stato solo se lo schermo NON è bloccato
void display_update_status(String text, RobotMood mood) {
  // Se c'è un messaggio importante in corso, IGNORA questo aggiornamento
  if (isMessageLocked) {
    return; 
  }
  currentText = text;
  currentMood = mood;
}

// 3. RESET FORZATO
void display_reset() {
  isMessageLocked = false;
  currentText = "Pronto";
  currentMood = MOOD_IDLE;
}

// LOOP DI ANIMAZIONE
void display_animate() {
  unsigned long now = millis();

  // --- CONTROLLO SCADENZA TIMER ---
  if (isMessageLocked && (now - messageTimer > messageDuration)) {
    isMessageLocked = false; // Sblocca
    currentMood = MOOD_IDLE; // Torna normale
    currentText = "Pronto";
  }

  display.clearDisplay();

  // 1. TESTO (Sotto)
  display.drawLine(0, 48, 128, 48, SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds(currentText, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 53);
  display.print(currentText);

  // 2. OCCHI (Sopra)
  // Logica Blink
  if (currentMood == MOOD_IDLE || currentMood == MOOD_LOOKING) {
    if (!isBlinking && (now - lastBlinkTime > blinkInterval)) {
      isBlinking = true; lastBlinkTime = now;
    } else if (isBlinking && (now - lastBlinkTime > 150)) {
      isBlinking = false; lastBlinkTime = now; blinkInterval = random(2000, 6000);
    }
  } else { isBlinking = false; }

  // Disegno Facce
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