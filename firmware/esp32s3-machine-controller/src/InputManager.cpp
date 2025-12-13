// src/InputManager.cpp
#include "InputManager.h"
#include "config.h"
#include <Arduino.h>

// Variabili statiche per il debounce (visibili solo in questo file)
static unsigned long lastConfirmPress = 0;
static unsigned long lastCorrectionPaperPress = 0;
static unsigned long lastCorrectionPlasticPress = 0;
static unsigned long lastCorrectionGlassPress = 0;
static unsigned long lastCancelPress = 0;

void input_setup()
{
  pinMode(CONFIRM_PIN, INPUT_PULLUP);
  pinMode(CORRECTION_PIN_PAPER, INPUT_PULLUP);
  pinMode(CORRECTION_PIN_PLASTIC, INPUT_PULLUP);
  pinMode(CORRECTION_PIN_GLASS, INPUT_PULLUP);
  pinMode(CANCEL_PIN, INPUT_PULLUP);
}

int checkButtons()
{
  unsigned long now = millis();
  int buttonPressed = BTN_NONE; // Nessun pulsante premuto di default

  // 1. Controlla Pulsante CONFERMA
  if (digitalRead(CONFIRM_PIN) == LOW)
  {
    if (now - lastConfirmPress > DEBOUNCE_DELAY)
    {
      buttonPressed = BTN_CONFIRM;
      lastConfirmPress = now;
    }
  }

  // 2. Controlla Pulsante CORREZIONE CARTA
  else if (digitalRead(CORRECTION_PIN_PAPER) == LOW)
  {
    if (now - lastCorrectionPaperPress > DEBOUNCE_DELAY)
    {
      buttonPressed = BTN_CORRECT_PAPER;
      lastCorrectionPaperPress = now;
    }
  }

  // 3. <<< Controlla Pulsante CORREZIONE PLASTICA >>>
  else if (digitalRead(CORRECTION_PIN_PLASTIC) == LOW)
  {
    if (now - lastCorrectionPlasticPress > DEBOUNCE_DELAY)
    {
      buttonPressed = BTN_CORRECT_PLASTIC;
      lastCorrectionPlasticPress = now;
    }
  }

  // 4. <<< Controlla Pulsante CORREZIONE VETRO >>>
  else if (digitalRead(CORRECTION_PIN_GLASS) == LOW)
  {
    if (now - lastCorrectionGlassPress > DEBOUNCE_DELAY)
    {
      buttonPressed = BTN_CORRECT_GLASS;
      lastCorrectionGlassPress = now;
    }
  }

  // 5. <<< Controlla Pulsante ANNULLA >>>
  else if (digitalRead(CANCEL_PIN) == LOW) {
    if (now - lastCancelPress > DEBOUNCE_DELAY) {
      buttonPressed = BTN_CANCEL;
      lastCancelPress = now;
    }
  }

  return buttonPressed;
}