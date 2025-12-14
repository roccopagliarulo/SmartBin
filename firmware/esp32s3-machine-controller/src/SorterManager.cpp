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
  
  Serial.println("SORTER: Setting all sorters to STOP (93).");
  servoPlastic.write(SORTER_POS_STOP);
  servoPaper.write(SORTER_POS_STOP);
  servoGlass.write(SORTER_POS_STOP);
  delay(500); 
}

// Private function to find the correct servo
static Servo* get_target_servo(String material) {
  if (material == "plastic" || material.equals("plastic_metal")) {
    Serial.println("SORTER: Target -> PLASTIC / METAL");
    return &servoPlastic;
  } 
  else if (material == "paper" || material.equals("paper_cardboard")) {
    Serial.println("SORTER: Target -> PAPER / CARDBOARD");
    return &servoPaper;
  }
  else if (material == "glass") {
    Serial.println("SORTER: Target -> GLASS");
    return &servoGlass;
  }
  else {
    Serial.println("SORTER: 'unknown' material, letting pass.");
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
    targetServo->write(SORTER_POS_STOP); // Return a STOP
  }
}

// Executes only the "FORWARD" movement and STOP
void sorter_reset(String material) {
  Servo* targetServo = get_target_servo(material);

  if (targetServo != NULL) {
    Serial.println("SORTER: FORWARD movement (50ms)...");
    targetServo->write(SORTER_VEL_INDIETRO);
    delay(SORTER_KICK_TIME); // 50ms
    targetServo->write(SORTER_POS_STOP); // return to STOP
  }
}