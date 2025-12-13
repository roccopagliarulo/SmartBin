// src/SorterManager.cpp
#include "SorterManager.h"
#include "config.h"
#include <ESP32Servo.h>

Servo servoPlastic;
Servo servoPaper;
Servo servoGlass;

void sorter_setup() {
  servoPlastic.attach(SORTER_PLASTIC_PIN);
  servoPaper.attach(SORTER_PAPER_PIN);
  servoGlass.attach(SORTER_GLASS_PIN);
  
  Serial.println("SORTER: Imposto tutti i sorter su STOP (93).");
  servoPlastic.write(SORTER_POS_STOP);
  servoPaper.write(SORTER_POS_STOP);
  servoGlass.write(SORTER_POS_STOP);
  delay(500); 
}

// Funzione privata per trovare il servo giusto
static Servo* get_target_servo(String material) {
  if (material == "plastic" || material.equals("plastic_metal")) {
    Serial.println("SORTER: Target -> PLASTIC");
    return &servoPlastic;
  } 
  else if (material == "paper" || material.equals("paper_cardboard")) {
    Serial.println("SORTER: Target -> PAPER");
    return &servoPaper;
  }
  else if (material == "glass") {
    Serial.println("SORTER: Target -> GLASS");
    return &servoGlass;
  }
  else {
    Serial.println("SORTER: Materiale 'unknown', lascio passare.");
    return NULL;
  }
}

// Esegue solo il movimento "AVANTI" e STOP
void sorter_activate(String material) {
  Servo* targetServo = get_target_servo(material);

  if (targetServo != NULL) {
    Serial.println("SORTER: Movimento AVANTI (50ms)...");
    targetServo->write(SORTER_VEL_AVANTI);
    delay(SORTER_KICK_TIME); // 50ms
    targetServo->write(SORTER_POS_STOP); // Torna a STOP
  }
}

// Esegue solo il movimento "INDIETRO" e STOP
void sorter_reset(String material) {
  Servo* targetServo = get_target_servo(material);

  if (targetServo != NULL) {
    Serial.println("SORTER: Movimento RESET (INDIETRO 50ms)...");
    targetServo->write(SORTER_VEL_INDIETRO);
    delay(SORTER_KICK_TIME); // 50ms
    targetServo->write(SORTER_POS_STOP); // Torna a STOP
  }
}