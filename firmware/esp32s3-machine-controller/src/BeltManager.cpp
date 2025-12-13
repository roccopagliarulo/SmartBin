// src/BeltManager.cpp
#include "BeltManager.h"
#include "config.h"
#include "NetworkManager.h" // Per inviare il messaggio "cleared"
#include "SorterManager.h"
#include <Arduino.h>

// Definiamo gli stati del nastro
enum BeltState {
  IDLE,   // 0: Fermo, ignora il sensore
  ARMED,  // 1: In attesa che l'oggetto cada sul sensore
  RUNNING // 2: Nastro in movimento, in attesa che l'oggetto sparisca
};

// --- Variabile per memorizzare il materiale sul nastro ---
String currentMaterialOnBelt = "unknown";


unsigned long beltStartTime = 0; // Variabile per memorizzare l'ora di avvio 
BeltState currentBeltState = IDLE;
bool lastSensorState = false; // Per rilevare solo i cambiamenti

// Funzioni private per controllare il motore
static void motor_start() {
  Serial.println("NASTRO: Avvio motore.");
  digitalWrite(BELT_MOTOR_IN1_PIN, HIGH);
  digitalWrite(BELT_MOTOR_IN2_PIN, LOW);
}

static void motor_stop() {
  Serial.println("NASTRO: Arresto motore.");
  digitalWrite(BELT_MOTOR_IN1_PIN, LOW);
  digitalWrite(BELT_MOTOR_IN2_PIN, LOW);
}

// Funzione pubblica di setup
void belt_setup() {
  pinMode(BELT_IR_SENSOR_PIN, INPUT);
  
  // Setup pin motore
  pinMode(BELT_MOTOR_IN1_PIN, OUTPUT);
  pinMode(BELT_MOTOR_IN2_PIN, OUTPUT);
  
  // Assicurati che il motore sia fermo all'avvio
  motor_stop();
  currentBeltState = IDLE;
}

// Funzione chiamata da NetworkManager quando arriva "open_gate"
void belt_arm_system(const char* material) {
  if (currentBeltState == IDLE) {
    Serial.println("NASTRO: Sistema ARMATO. In attesa di oggetto...");
    Serial.println(material);

    currentMaterialOnBelt = String(material);
    currentBeltState = ARMED;
    lastSensorState = false; // Resetta lo stato del sensore
  } else {
    Serial.println("NASTRO: Errore, tentato di armare un sistema non IDLE.");
  }
}

// Funzione privata per leggere il sensore
bool is_object_detected_on_belt() {
  return (digitalRead(BELT_IR_SENSOR_PIN) == LOW);
}

// Funzione pubblica di loop (il cuore della logica)
void belt_loop() {
  // Leggi il sensore solo se ARMED o RUNNING
  if (currentBeltState == IDLE) {
    return; // Non fare nulla se siamo inattivi (ignora il sensore)
  }
  // ***************************************

  bool objectDetected = is_object_detected_on_belt();
  bool stateChanged = (objectDetected != lastSensorState);
  lastSensorState = objectDetected;

  switch (currentBeltState) {
    case ARMED:
      // Se lo stato è ARMED, aspettiamo che l'oggetto appaia
      if (objectDetected && stateChanged) {
        Serial.println("NASTRO: Eseguo movimento sorter INIZIALE.");
        sorter_activate(currentMaterialOnBelt);
        motor_start();
        beltStartTime = millis();
        currentBeltState = RUNNING;
      }
      break;

    case RUNNING:
      // Se lo stato è RUNNING, aspettiamo che l'oggetto SPARISCA
      if (millis() - beltStartTime >= BELT_RUN_TIME) {
        
        Serial.println("NASTRO: Tempo scaduto! Arresto nastro.");
        currentBeltState = IDLE;
        // 1. Ferma il motore
        motor_stop();

        // 2. Esegui "movimento di reset" (INDIETRO 50ms -> STOP)
        Serial.println("NASTRO: Eseguo movimento sorter RESET.");
        sorter_reset(currentMaterialOnBelt);

        // 3. Imposta lo stato su IDLE e pulisci
        currentBeltState = IDLE;
        currentMaterialOnBelt = "unknown";
        
        // --- INVIA IL MESSAGGIO A NODE-RED ---
        Serial.println("NASTRO: Invio 'cleared' a Node-RED.");
        char payload[30];
        snprintf(payload, sizeof(payload), "{\"status\":\"cleared\"}");
        // Usiamo il client MQTT dal NetworkManager
        client.publish("smartbin/belt/event/cleared", payload);
      }
      break;
    
    case IDLE:
      // Non dovrebbe mai arrivare qui
      break;
  }
}