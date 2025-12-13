// src/main.cpp
#include <Arduino.h>
#include "config.h"
#include "NetworkManager.h"
#include "ProximitySensor.h"
#include "InputManager.h"
#include "DisplayManager.h"
#include "ServoManager.h"
#include "BeltManager.h"
#include "SorterManager.h"

// Variabile di stato per la prossimità
int lastDetectionState = 0;

bool lastBeltSensorState = false;

void setup()
{
  Serial.begin(115200);

  // Inizializza tutti i moduli
  display_setup();
  proximity_setup();
  input_setup();
  network_setup(); // Questa si connette al WiFi e imposta MQTT

  // --- Setup Servo ---
  servo_setup();
  servo_reset_position(); // Assicura che il cancello sia chiuso all'avvio

  // --- Gestore del nastro ---
  belt_setup();
  sorter_setup();

  Serial.println("ESP32-S3 Mouth Controller Avviato.");
  Serial.print("Topic Prossimità: ");
  Serial.println(MQTT_TOPIC_PROXIMITY);
  Serial.print("Topic Conferma: ");
  Serial.println(MQTT_TOPIC_CONFIRM);
  Serial.print("Topic Correzione: ");
  Serial.println(MQTT_TOPIC_CORRECTION);
  Serial.print("Distanza Trigger: < ");
  Serial.print(TRIGGER_DISTANCE_CM);
  Serial.println(" cm");

}


void loop()
{
  network_loop();
  display_animate(); // FONDAMENTALE: Tienilo sempre attivo

  // --- 1. Logica Sensore (BASSA PRIORITÀ) ---
  int distance = getDistance();
  int currentDetectionState = (distance > 2 && distance < TRIGGER_DISTANCE_CM) ? 1 : 0;

  if (currentDetectionState != lastDetectionState)
  {
    if (currentDetectionState == 1)
    {
      char payload[30];
      snprintf(payload, sizeof(payload), "{\"user\":\"detected\"}");
      client.publish(MQTT_TOPIC_PROXIMITY, payload);

      // Usa update_status: se c'è un messaggio importante (es. Correzione), questo viene IGNORATO
      display_update_status("Classifico...", MOOD_LOOKING);
    }
    else
    {
      // Quando l'utente se ne va, prova a tornare a Pronto
      display_update_status("Pronto", MOOD_IDLE);
    }
    lastDetectionState = currentDetectionState;
  }

  // --- 2. Logica Pulsanti (ALTA PRIORITÀ) ---
  int buttonPressed = checkButtons();

  switch (buttonPressed)
  {
  case BTN_CONFIRM:
    client.publish(MQTT_TOPIC_CONFIRM, "{\"confirm\":true}");
    // Mostra per 2 secondi, bloccando il sensore
    display_show_temporary("GRAZIE!", MOOD_HAPPY, 2000); 
    break;

  case BTN_CANCEL:
    client.publish(MQTT_TOPIC_CANCEL, "{\"action\":\"cancel\"}");
    display_show_temporary("ANNULLATO", MOOD_SAD, 2000);
    break;

  case BTN_CORRECT_PAPER:
    client.publish(MQTT_TOPIC_CORRECTION, "{\"corrected_class\":\"paper_cardboard\"}");
    display_show_temporary("CARTA", MOOD_HAPPY, 2000);
    break;

  case BTN_CORRECT_PLASTIC:
    client.publish(MQTT_TOPIC_CORRECTION, "{\"corrected_class\":\"plastic_metal\"}");
    display_show_temporary("PLASTICA", MOOD_HAPPY, 2000);
    break;

  case BTN_CORRECT_GLASS:
    client.publish(MQTT_TOPIC_CORRECTION, "{\"corrected_class\":\"glass\"}");
    display_show_temporary("VETRO", MOOD_HAPPY, 2000);
    break;
  }

  belt_loop();
  // Rimuovi delay(20) se puoi, o tienilo molto basso
}


// void loop()
// {
//   // Gestisce la connessione MQTT (riconnessione, ecc.)
//   network_loop();

//   display_animate();

//   // --- 1. Logica Sensore ---
//   int distance = getDistance();
//   int currentDetectionState = (distance > 2 && distance < TRIGGER_DISTANCE_CM) ? 1 : 0;

//   if (currentDetectionState != lastDetectionState)
//   {
//     if (currentDetectionState == 1)
//     {
//       Serial.println("Oggetto Rilevato! Invio MQTT...");
//       display_set_mood(MOOD_LOOKING);
//       char payload[30];
//       snprintf(payload, sizeof(payload), "{\"user\":\"detected\"}");

//       // Usa le variabili globali definite in NetworkManager
//       client.publish(MQTT_TOPIC_PROXIMITY, payload);

//     }
//     else
//     {
//       display_set_mood(MOOD_IDLE);
//       Serial.println("Libero.");
//     }
//     lastDetectionState = currentDetectionState;
//   }

//   // --- 2. Logica Pulsanti ---
//   int buttonPressed = checkButtons();

//   switch (buttonPressed)
//   {
//   // CASO CONFERMA
//   case BTN_CONFIRM:
//   {
//     Serial.println("BTN: CONFERMA premuto.");
//     char payload[30];
//     snprintf(payload, sizeof(payload), "{\"confirm\":true}");
//     client.publish(MQTT_TOPIC_CONFIRM, payload);
//     display_set_mood(MOOD_HAPPY); // Faccia felice
//     display_set_text("GRAZIE!");
//     break;
//   }

//   case BTN_CANCEL:
//   {
//     Serial.println("BTN: ANNULLA premuto.");
//     char payload[30];
//     snprintf(payload, sizeof(payload), "{\"action\":\"cancel\"}");
//     display_set_mood(MOOD_SAD);
//     display_set_text("ANNULLATO");

//     delay(1000);

//     display_set_mood(MOOD_IDLE);
//     display_set_text("Pronto");

//     client.publish(MQTT_TOPIC_CANCEL, payload);
//     display_set_mood(MOOD_IDLE); // Torna normale
    

//     // Feedback immediato locale
//     delay(1000);

//     break;
//   }

//   // CASO CARTA
//   case BTN_CORRECT_PAPER:
//   {
//     Serial.println("BTN: CORREZIONE (Paper) premuto.");
//     char payload[60];
//     // Nota: Assicurati che "paper_cardboard" corrisponda al tuo switch su Node-RED
//     snprintf(payload, sizeof(payload), "{\"corrected_class\":\"paper_cardboard\"}");
//     client.publish(MQTT_TOPIC_CORRECTION, payload);
//     display_set_mood(MOOD_HAPPY); // Feedback positivo
//     display_set_text("MOD: CARTA");
//     break;
//   }

//   // CASO PLASTICA
//   case BTN_CORRECT_PLASTIC:
//   {
//     Serial.println("BTN: CORREZIONE (Plastic) premuto.");
//     char payload[60];
//     // Nota: "plastic_metal" deve corrispondere al tuo switch su Node-RED
//     snprintf(payload, sizeof(payload), "{\"corrected_class\":\"plastic_metal\"}");
//     client.publish(MQTT_TOPIC_CORRECTION, payload);
//     display_set_mood(MOOD_HAPPY); // Feedback positivo
//     display_set_text("MOD: PLASTICA");
//     break;
//   }

//   // CASO VETRO
//   case BTN_CORRECT_GLASS:
//   {
//     Serial.println("BTN: CORREZIONE (Glass) premuto.");
//     char payload[60];
//     // Nota: "glass" deve corrispondere al tuo switch su Node-RED
//     snprintf(payload, sizeof(payload), "{\"corrected_class\":\"glass\"}");
//     client.publish(MQTT_TOPIC_CORRECTION, payload);
//     // display_show_message("Corretto!", "VETRO");
//     display_set_mood(MOOD_HAPPY); // Feedback positivo
//     display_set_text("MOD: VETRO");
//     break;
//   }

//   case BTN_NONE:
//     break;
//   }

//   // 3. Gestisce la macchina a stati del nastro (IDLE, ARMED, RUNNING)
//   belt_loop();

//   // Delay per la reattività
//   delay(20);
// }