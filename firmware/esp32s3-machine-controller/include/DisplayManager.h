#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>

// Robot moods for eye animations
enum RobotMood {
    MOOD_SLEEP,     // Off / Sleeping
    MOOD_IDLE,      // Normal, looking around
    MOOD_LOOKING,   // Attentive (Proximity active)
    MOOD_THINKING,  // Processing (or waiting for classification)
    MOOD_HAPPY,     // Success / Confirmation
    MOOD_SAD,       // Cancel / Error
    MOOD_CONFUSED   // Unknown / Not understood
};

// --- Setup and Loop Functions ---
void display_setup();
void display_animate(); // Da chiamare SEMPRE nel loop per aggiornare occhi e timer

// --- Message Control Functions ---

// 1. HIGH PRIORITY: Show a message and LOCK the screen for 'duration' milliseconds.
// Useful for: Buttons, Errors, Confirmations. Prevents sensor from overwriting.
void display_show_temporary(String text, RobotMood mood, int duration);

// 2. LOW PRIORITY: Update status only if the screen is NOT locked.
// Useful for: Proximity sensor, Idle state.
void display_update_status(String text, RobotMood mood);

// 3. RESET: Force unlock the screen and return to IDLE/Ready.
// Useful for: End of operations (e.g., after gate opening).
void display_reset();

// --- Helper Functions (internal or manual use) ---
void display_set_mood(RobotMood mood);
void display_set_text(String text);

#endif