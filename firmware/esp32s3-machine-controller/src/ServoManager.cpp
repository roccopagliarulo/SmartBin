// src/ServoManager.cpp
#include "ServoManager.h"
#include "config.h"
#include <ESP32Servo.h>
#include <Arduino.h>

// Create Servo object
Servo gateServo;

void servo_setup() {
  // Connect the servo to the defined pin
  gateServo.attach(SERVO_PIN);
}

void servo_reset_position() {
  // At startup, attach servo and set to STOP position
  gateServo.attach(SERVO_PIN);
  delay(50);
  
  Serial.println("Servo: Setting STOP position.");
  gateServo.write(SERVO_POS_STOP);
   // Don't detach the pin to maintain "brake"
  // and prevent creeping.
  // gateServo.detach(); // Commented out
}

void servo_open_gate() {
  // This function now executes a timed sequence
  
  // 1. Make sure pin is attached
  gateServo.attach(SERVO_PIN);
  delay(50); 

  Serial.println("Servo: Starting OPEN rotation...");
  
  // 2. Rotate at OPEN speed for defined time
  gateServo.write(SERVO_VEL_APERTURA);
  delay(SERVO_TEMPO_MOVIMENTO); 

  // 3. STOP (to give time for waste to fall)
  Serial.println("Servo: STOP (waiting for waste to fall)...");
  gateServo.write(SERVO_POS_STOP);
  delay(1500); // 1.5 sec pause

  // 4. Rotate at CLOSE speed for defined time
  Serial.println("Servo: Starting CLOSE rotation...");
  gateServo.write(SERVO_VEL_CHIUSURA);
  delay(SERVO_TEMPO_MOVIMENTO);
  
  // 5. STOP again (return to rest position)
  Serial.println("Servo: STOP (rest).");
  gateServo.write(SERVO_POS_STOP);
}