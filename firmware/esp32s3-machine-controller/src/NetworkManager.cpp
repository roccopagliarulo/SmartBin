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

// Define global variables
WiFiClient espClient;
PubSubClient client(espClient);
char MQTT_TOPIC_PROXIMITY[100];
char MQTT_TOPIC_CONFIRM[100];
char MQTT_TOPIC_CORRECTION[100];
char MQTT_TOPIC_UI_CLASSIFY[100];
char MQTT_TOPIC_COMMAND[100];
char MQTT_TOPIC_CANCEL[100];

// "Private" functions (static) for this file
static void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received [");
  Serial.print(topic);
  Serial.print("] ");

  // Convert payload to String
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  String messageStr = String(message);
  
  Serial.println(messageStr);

  // Check if classification topic
  if (strcmp(topic, MQTT_TOPIC_UI_CLASSIFY) == 0) {
    display_set_text(messageStr);
    // If we receive "unknown", show confused face
    if (messageStr.indexOf("unknown") >= 0 || messageStr.indexOf("NON RICONOSCIUTO") >= 0) {
        display_set_mood(MOOD_CONFUSED);
    } 
    // Otherwise we're thinking/waiting for confirmation
    else {
        display_set_mood(MOOD_THINKING); 
    }
    
    // Also show text for clarity (optional)
    // display_show_text(messageStr);
  
    // Check if command topic
  } else if (strcmp(topic, MQTT_TOPIC_COMMAND) == 0) {
    
    // Parse JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, messageStr);

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }

    // Check which action to perform
    const char* action = doc["action"];
    if (strcmp(action, "open_gate") == 0) {
      Serial.println("Received 'open_gate' command. Starting servo.");
      display_set_mood(MOOD_HAPPY);
      display_set_text("OPENING...");
      // Call our new manager
      // 1. Open the gate
      servo_open_gate(); 
      // 2. Arm the belt system
      const char* material = doc["target"];
      if (material == NULL) {
          material = "unknown"; // Sicurezza
      }
      // 3. Set happy face and text
      // Use direct function as we'll block the loop
      display_set_mood(MOOD_HAPPY);
      display_set_text("OPENING..."); 
      display_animate(); // Force immediate display update
      
      // 4. Arm the belt (passing material)
      belt_arm_system(material);
      // 4. RESET DISPLAY AT THE END!
      // Without this, "OPENING..." stays forever
      display_reset(); 
      display_animate(); // Immediately update to "Ready"
    }


    // Note: will ignore "take_photo" command, which is correct.
  }
}


static void reconnect_mqtt() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "esp32s3-mouth-" + String(MOUTH_ID);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");

      // --- Iscriviti al topic della UI ---
      Serial.print("Subscribing to: ");
      Serial.println(MQTT_TOPIC_UI_CLASSIFY);
      client.subscribe(MQTT_TOPIC_UI_CLASSIFY);

      // --- Iscrizione al topic dei comandi ---
      Serial.print("Subscribing to: ");
      Serial.println(MQTT_TOPIC_COMMAND);
      client.subscribe(MQTT_TOPIC_COMMAND);
      // ------------------------------------------



    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 5 seconds");
      delay(5000);
    }
  }
}

// Public functions
void network_setup() {
  setup_wifi();

  client.setServer(MQTT_BROKER_IP, MQTT_PORT);
  client.setCallback(mqtt_callback);

  client.setServer(MQTT_BROKER_IP, MQTT_PORT);

  // Build topic names
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