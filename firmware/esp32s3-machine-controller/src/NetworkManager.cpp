// src/NetworkManager.cpp
#include "NetworkManager.h"
#include "config.h"
#include <WiFi.h>
#include <Arduino.h>
#include "DisplayManager.h"
#include <ArduinoJson.h>
#include "ServoManager.h"
#include "BeltManager.h"
#include "SorterManager.h"

// Definiamo le variabili globali
WiFiClient espClient;
PubSubClient client(espClient);
char MQTT_TOPIC_PROXIMITY[100];
char MQTT_TOPIC_CONFIRM[100];
char MQTT_TOPIC_CORRECTION[100];
char MQTT_TOPIC_UI_CLASSIFY[100];
char MQTT_TOPIC_COMMAND[100];
char MQTT_TOPIC_CANCEL[100];

// Funzioni "private" (static) per questo file
static void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connessione a ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connesso");
  Serial.println("Indirizzo IP: ");
  Serial.println(WiFi.localIP());
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Messaggio ricevuto [");
  Serial.print(topic);
  Serial.print("] ");

  // Converti il payload in una Stringa
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  String messageStr = String(message);
  
  Serial.println(messageStr);

  // Controlla se è il topic della classificazione
  if (strcmp(topic, MQTT_TOPIC_UI_CLASSIFY) == 0) {
    display_set_text(messageStr);
    // Se riceviamo "unknown", faccia confusa
    if (messageStr.indexOf("unknown") >= 0 || messageStr.indexOf("NON RICONOSCIUTO") >= 0) {
        display_set_mood(MOOD_CONFUSED);
    } 
    // Altrimenti stiamo pensando/attendendo conferma
    else {
        display_set_mood(MOOD_THINKING); 
    }
    
    // Mostriamo anche il testo per chiarezza (opzionale)
    // display_show_text(messageStr);
  
    // --- Controlla se è il topic dei comandi ---
  } else if (strcmp(topic, MQTT_TOPIC_COMMAND) == 0) {
    
    // Analizza il JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, messageStr);

    if (error) {
      Serial.print("deserializeJson() fallito: ");
      Serial.println(error.c_str());
      return;
    }

    // Controlla quale azione eseguire
    const char* action = doc["action"];
    if (strcmp(action, "open_gate") == 0) {
      Serial.println("Ricevuto comando 'open_gate'. Avvio servo.");
      display_set_mood(MOOD_HAPPY);
      display_set_text("APERTURA...");
      // Chiama il nostro nuovo gestore
      // 1. Apri il cancello
      servo_open_gate(); 
      // 2. Arma il sistema nastro >>>
      const char* material = doc["target"];
      if (material == NULL) {
          material = "unknown"; // Sicurezza
      }
      // 3. Imposta faccia Felice e Testo
      // Usiamo una funzione diretta perché stiamo per bloccare il loop
      display_set_mood(MOOD_HAPPY);
      display_set_text("APERTURA..."); 
      display_animate(); // <--- FORZA L'AGGIORNAMENTO GRAFICO ORA!
      
      // 4. <<< Arma il nastro (passando il materiale) >>>
      belt_arm_system(material);
      // 4. RESETTA IL DISPLAY ALLA FINE!
      // Se non metti questo, rimane "APERTURA..." per sempre
      display_reset(); 
      display_animate(); // Aggiorna subito a "Pronto"
    }


    // Nota: ignorerà il comando "take_photo", che è corretto.
  }
}


static void reconnect_mqtt() {
  while (!client.connected()) {
    Serial.print("Tentativo di connessione MQTT...");
    String clientId = "esp32s3-mouth-" + String(MOUTH_ID);
    if (client.connect(clientId.c_str())) {
      Serial.println("connesso");

      // --- Iscriviti al topic della UI ---
      Serial.print("Iscrizione a: ");
      Serial.println(MQTT_TOPIC_UI_CLASSIFY);
      client.subscribe(MQTT_TOPIC_UI_CLASSIFY);

      // --- Iscrizione al topic dei comandi ---
      Serial.print("Iscrizione a: ");
      Serial.println(MQTT_TOPIC_COMMAND);
      client.subscribe(MQTT_TOPIC_COMMAND);
      // ------------------------------------------



    } else {
      Serial.print("fallito, rc=");
      Serial.print(client.state());
      Serial.println(" riprovo tra 5 secondi");
      delay(5000);
    }
  }
}

// Funzioni "pubbliche"
void network_setup() {
  setup_wifi();

  client.setServer(MQTT_BROKER_IP, MQTT_PORT);
  client.setCallback(mqtt_callback);

  client.setServer(MQTT_BROKER_IP, MQTT_PORT);

  // Costruisci i nomi dei topic
  snprintf(MQTT_TOPIC_PROXIMITY, sizeof(MQTT_TOPIC_PROXIMITY), 
           "smartbin/mouth/%s/event/proximity", MOUTH_ID);
           
  snprintf(MQTT_TOPIC_CONFIRM, sizeof(MQTT_TOPIC_CONFIRM), 
           "smartbin/mouth/%s/event/confirm", MOUTH_ID);

  snprintf(MQTT_TOPIC_CORRECTION, sizeof(MQTT_TOPIC_CORRECTION), 
           "smartbin/mouth/%s/event/correction", MOUTH_ID);

  snprintf(MQTT_TOPIC_CANCEL, sizeof(MQTT_TOPIC_CANCEL), 
           "smartbin/mouth/%s/event/cancel", MOUTH_ID);

  snprintf(MQTT_TOPIC_UI_CLASSIFY, sizeof(MQTT_TOPIC_UI_CLASSIFY), 
           "smartbin/ui/classification"); // Nota: questo topic è generico, non per-bocca

  snprintf(MQTT_TOPIC_COMMAND, sizeof(MQTT_TOPIC_COMMAND), 
           "smartbin/mouth/%s/command", MOUTH_ID);
}
void network_loop() {
  if (!client.connected()) {
    reconnect_mqtt();
  }
  client.loop();
}