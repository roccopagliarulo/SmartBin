// include/BeltManager.h
#pragma once

// Funzione di inizializzazione
void belt_setup();

// Funzione che controlla il sensore
bool is_object_detected_on_belt();

void belt_loop(); // Questa funzione gestirà la macchina a stati

// Funzione chiamata dal NetworkManager per "armare" il nastro
void belt_arm_system(const char* material);