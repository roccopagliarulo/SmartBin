// src/BeltManager.cpp
#include "BeltManager.h"
#include "config.h"
#include "NetworkManager.h" // For sending the "cleared" message
#include "SorterManager.h"
#include <Arduino.h>

// Define belt states
enum BeltState {
  IDLE,   // 0: Stopped, ignore sensor
  ARMED,  // 1: Waiting for object to reach sensor
  RUNNING // 2: Belt moving, waiting for object to disappear
};

// Variable to store material on belt
String currentMaterialOnBelt = "unknown";


unsigned long beltStartTime = 0; // Variable to store start time
BeltState currentBeltState = IDLE;
bool lastSensorState = false; // For detecting changes only

// Private functions to control motor
static void motor_start() {
  Serial.println("BELT: Starting motor.");
  digitalWrite(BELT_MOTOR_IN1_PIN, HIGH);
  digitalWrite(BELT_MOTOR_IN2_PIN, LOW);
}

static void motor_stop() {
  Serial.println("BELT: Stopping motor.");
  digitalWrite(BELT_MOTOR_IN1_PIN, LOW);
  digitalWrite(BELT_MOTOR_IN2_PIN, LOW);
}

// Public setup function
void belt_setup() {
  pinMode(BELT_IR_SENSOR_PIN, INPUT);
  
  // Setup motor control pins
  pinMode(BELT_MOTOR_IN1_PIN, OUTPUT);
  pinMode(BELT_MOTOR_IN2_PIN, OUTPUT);
  
  // Ensure motor is stopped at startup
  motor_stop();
  currentBeltState = IDLE;
}

// Function called by NetworkManager when "open_gate" arrives
void belt_arm_system(const char* material) {
  if (currentBeltState == IDLE) {
    Serial.println("BELT: System ARMED. Waiting for object...");
    Serial.println(material);

    currentMaterialOnBelt = String(material);
    currentBeltState = ARMED;
    lastSensorState = false; // Reset sensor state
  } else {
    Serial.println("BELT: Error, attempted to arm non-IDLE system.");
  }
}

// private function to read IR sensor
bool is_object_detected_on_belt() {
  return (digitalRead(BELT_IR_SENSOR_PIN) == LOW);
}

// Public loop function (core logic)
void belt_loop() {
   // Only read sensor if ARMED or RUNNING
  if (currentBeltState == IDLE) {
    return; // Do nothing if idle (ignore sensor)
  }
  // ***************************************

  bool objectDetected = is_object_detected_on_belt();
  bool stateChanged = (objectDetected != lastSensorState);
  lastSensorState = objectDetected;

  switch (currentBeltState) {
    case ARMED:
      // If ARMED, wait for object to appear
      if (objectDetected && stateChanged) {
        Serial.println("BELT: Executing initial sorter movement.");
        sorter_activate(currentMaterialOnBelt);
        motor_start();
        beltStartTime = millis();
        currentBeltState = RUNNING;
      }
      break;

    case RUNNING:
      // If RUNNING, wait for object to DISAPPEAR
      if (millis() - beltStartTime >= BELT_RUN_TIME) {
        
        Serial.println("BELT: Timeout! Stopping belt.");
        currentBeltState = IDLE;
        // 1. Stop the belt motor
        motor_stop();

        // 2. Execute "reset movement" (BACKWARD 50ms -> STOP)
        Serial.println("BELT: Executing sorter RESET movement.");
        sorter_reset(currentMaterialOnBelt);

        // 3. Set state to IDLE and clean up
        currentBeltState = IDLE;
        currentMaterialOnBelt = "unknown";
        
        // Send message to Node-RED
        Serial.println("BELT: Sending 'cleared' to Node-RED.");
        char payload[30];
        snprintf(payload, sizeof(payload), "{\"status\":\"cleared\"}");
        // Use MQTT client from NetworkManager
        client.publish("smartbin/belt/event/cleared", payload);
      }
      break;
    
    case IDLE:
      // Should never get here
      break;
  }
}