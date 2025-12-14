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

// State variable for proximity
int lastDetectionState = 0;

bool lastBeltSensorState = false;

void setup()
{
  Serial.begin(115200);

  // Initialize all modules
  display_setup();
  proximity_setup();
  input_setup();
  network_setup(); // This connects to WiFi and sets up MQTT

  // --- Setup Servo ---
  servo_setup();
  servo_reset_position(); // Ensure gate is closed at startup

  // Belt manager setup
  belt_setup();
  sorter_setup();

  Serial.println("ESP32-S3 Mouth Controller Started.");
  Serial.print("Proximity Topic: ");
  Serial.println(MQTT_TOPIC_PROXIMITY);
  Serial.print("Confirm Topic: ");
  Serial.println(MQTT_TOPIC_CONFIRM);
  Serial.print("Correction Topic: ");
  Serial.println(MQTT_TOPIC_CORRECTION);
  Serial.print("Trigger Distance: < ");
  Serial.print(TRIGGER_DISTANCE_CM);
  Serial.println(" cm");

}


void loop()
{
  network_loop();
  display_animate(); // ESSENTIAL: Keep this always active

  // 1. Sensor Logic (LOW PRIORITY)
  int distance = getDistance();
  int currentDetectionState = (distance > 2 && distance < TRIGGER_DISTANCE_CM) ? 1 : 0;

  if (currentDetectionState != lastDetectionState)
  {
    if (currentDetectionState == 1)
    {
      char payload[30];
      snprintf(payload, sizeof(payload), "{\"user\":\"detected\"}");
      client.publish(MQTT_TOPIC_PROXIMITY, payload);

      // Use update_status: if there's an important message (e.g., Correction), this is IGNORED
      display_update_status("Classifying...", MOOD_LOOKING);
    }
    else
    {
      // When user leaves, try to return to Ready
      display_update_status("Ready", MOOD_IDLE);
    }
    lastDetectionState = currentDetectionState;
  }

  // 2. Button Logic (HIGH PRIORITY)
  int buttonPressed = checkButtons();

  switch (buttonPressed)
  {
  case BTN_CONFIRM:
    client.publish(MQTT_TOPIC_CONFIRM, "{\"confirm\":true}");
    // Show for 2 seconds, blocking the sensor
    display_show_temporary("THANK YOU!", MOOD_HAPPY, 2000); 
    break;

  case BTN_CANCEL:
    client.publish(MQTT_TOPIC_CANCEL, "{\"action\":\"cancel\"}");
    display_show_temporary("CANCELLED", MOOD_SAD, 2000);
    break;

  case BTN_CORRECT_PAPER:
    client.publish(MQTT_TOPIC_CORRECTION, "{\"corrected_class\":\"paper_cardboard\"}");
    display_show_temporary("PAPER or CARDBOARD", MOOD_HAPPY, 2000);
    break;

  case BTN_CORRECT_PLASTIC:
    client.publish(MQTT_TOPIC_CORRECTION, "{\"corrected_class\":\"plastic_metal\"}");
    display_show_temporary("PLASTIC or METAL", MOOD_HAPPY, 2000);
    break;

  case BTN_CORRECT_GLASS:
    client.publish(MQTT_TOPIC_CORRECTION, "{\"corrected_class\":\"glass\"}");
    display_show_temporary("GLASS", MOOD_HAPPY, 2000);
    break;
  }

  belt_loop();
}