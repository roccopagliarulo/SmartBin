// include/config.h
#pragma once // Prevent multiple inclusions

#define WIFI_SSID "WIFI_SSID"
#define WIFI_PASS "WIFI_PASSWORD"

#define MQTT_BROKER_IP "IP" 
#define MQTT_PORT 1883
#define MOUTH_ID "1"

// --- Sensor PIN Configuration ---
#define TRIG_PIN 4
#define ECHO_PIN 5
#define TRIGGER_DISTANCE_CM 10 

// --- Button PIN Configuration ---
#define CONFIRM_PIN 12
#define CORRECTION_PIN_PAPER 14
#define CORRECTION_PIN_PLASTIC 10
#define CORRECTION_PIN_GLASS 11

// --- Cancel Button ---
#define CANCEL_PIN 47

// --- Belt Sensor Configuration ---
#define BELT_IR_SENSOR_PIN 13

// --- Belt Motor Configuration (L298N) ---
#define BELT_MOTOR_IN1_PIN 35  // Pin Direzione 1
#define BELT_MOTOR_IN2_PIN 36  // Pin Direzione 2
#define BELT_RUN_TIME 12000 

// --- Debounce Configuration ---
#define DEBOUNCE_DELAY 50 // 50ms


// --- Gate Configuration ---
#define SERVO_PIN 17
#define SERVO_POS_STOP 93           // The stop point you found!
#define SERVO_VEL_APERTURA 40       // Speed in one direction (0 to 180)
#define SERVO_VEL_CHIUSURA 170      // Speed in the other direction
#define SERVO_TEMPO_MOVIMENTO 3100  // Time to open/close


// --- Sorter Servo Configuration ---

#define SORTER_PLASTIC_PIN 40
#define SORTER_PAPER_PIN 41
#define SORTER_GLASS_PIN 42

#define SORTER_POS_STOP 93
#define SORTER_VEL_AVANTI 180       // Forward speed
#define SORTER_VEL_INDIETRO 0       // Backward speed
#define SORTER_KICK_TIME 280        // 50ms (Note: Comment seems inconsistent with value)