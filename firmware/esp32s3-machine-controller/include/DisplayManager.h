#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>

// Stati d'animo del Robot per le animazioni degli occhi
enum RobotMood {
    MOOD_SLEEP,     // Spento / Dorme
    MOOD_IDLE,      // Normale, guarda in giro
    MOOD_LOOKING,   // Attento (Proximity attiva)
    MOOD_THINKING,  // Sta elaborando (o attesa classificazione)
    MOOD_HAPPY,     // Successo / Conferma
    MOOD_SAD,       // Annulla / Errore
    MOOD_CONFUSED   // Unknown / Non capito
};

// --- Funzioni di Setup e Ciclo ---
void display_setup();
void display_animate(); // Da chiamare SEMPRE nel loop per aggiornare occhi e timer

// --- Funzioni di Controllo Messaggi ---

// 1. ALTA PRIORITÀ: Mostra un messaggio e BLOCCA lo schermo per 'duration' millisecondi.
// Utile per: Pulsanti, Errori, Conferme. Impedisce al sensore di sovrascrivere.
void display_show_temporary(String text, RobotMood mood, int duration);

// 2. BASSA PRIORITÀ: Aggiorna lo stato solo se lo schermo NON è bloccato.
// Utile per: Sensore di prossimità, stato Idle.
void display_update_status(String text, RobotMood mood);

// 3. RESET: Sblocca forzatamente lo schermo e torna a IDLE/Pronto.
// Utile per: Fine operazioni (es. dopo apertura cancello).
void display_reset();

// --- Funzioni Helper (uso interno o manuale) ---
void display_set_mood(RobotMood mood);
void display_set_text(String text);

#endif