// include/SorterManager.h
#pragma once
#include <Arduino.h>

void sorter_setup();
void sorter_activate(String material); // Exetutes only the "FORWARD" movement and STOP
void sorter_reset(String material);    // Executes only the "BACKWARD" movement and STOP