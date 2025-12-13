// src/ServoManager.cpp
#include "ServoManager.h"
#include "config.h"
#include <ESP32Servo.h>
#include <Arduino.h>

// Crea l'oggetto Servo
Servo gateServo;

void servo_setup() {
  // Collega il servo al pin definito nel config
  gateServo.attach(SERVO_PIN);
}

void servo_reset_position() {
  // All'avvio, attacca il servo e digli di stare fermo.
  gateServo.attach(SERVO_PIN);
  delay(50);
  
  Serial.println("Servo: Imposto posizione STOP.");
  gateServo.write(SERVO_POS_STOP);
  // Non sganciamo il pin, così mantiene il "freno"
  // e non striscia (creeping).
  // gateServo.detach(); // Commentato via
}

void servo_open_gate() {
  // Questa funzione ora esegue una sequenza temporizzata
  
  // 1. Assicurati che il pin sia collegato
  gateServo.attach(SERVO_PIN);
  delay(50); 

  Serial.println("Servo: Avvio rotazione APERTURA...");
  
  // 2. Gira a velocità di APERTURA per il tempo definito
  gateServo.write(SERVO_VEL_APERTURA);
  delay(SERVO_TEMPO_MOVIMENTO); 

  // 3. FERMATI (per dare tempo al rifiuto di cadere)
  Serial.println("Servo: STOP (attesa caduta rifiuto)...");
  gateServo.write(SERVO_POS_STOP);
  delay(1500); // Pausa di 1.5 sec

  // 4. Gira a velocità di CHIUSURA per il tempo definito
  Serial.println("Servo: Avvio rotazione CHIUSURA...");
  gateServo.write(SERVO_VEL_CHIUSURA);
  delay(SERVO_TEMPO_MOVIMENTO);
  
  // 5. FERMATI di nuovo (torna alla posizione di riposo)
  Serial.println("Servo: STOP (riposo).");
  gateServo.write(SERVO_POS_STOP);
}