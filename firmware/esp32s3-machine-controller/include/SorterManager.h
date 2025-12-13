// include/SorterManager.h
#pragma once
#include <Arduino.h>

void sorter_setup();
void sorter_activate(String material); // Esegue solo il movimento "AVANTI" e STOP
void sorter_reset(String material);    // Esegue solo il movimento "INDIETRO" e STOP