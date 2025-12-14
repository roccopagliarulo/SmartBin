// include/BeltManager.h
#pragma once

// Initialization function
void belt_setup();

// Function to check the sensor
bool is_object_detected_on_belt();

void belt_loop(); // This function will handle the state machine

// Function called by NetworkManager to "arm" the belt system
void belt_arm_system(const char* material);