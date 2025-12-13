// src/ProximitySensor.cpp
#include "ProximitySensor.h"
#include "config.h" // Carica i nostri PIN
#include <Arduino.h>

void proximity_setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

int getDistance() {
  long duration;
  int distance;
  
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.0343 / 2;
  return distance;
}