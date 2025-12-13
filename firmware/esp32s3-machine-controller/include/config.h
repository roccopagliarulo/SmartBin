// include/config.h
#pragma once // Evita inclusioni multiple

#define WIFI_SSID "WIFI_SSID"
#define WIFI_SSID "WIFI_SSID"

#define MQTT_BROKER_IP "IP" 
#define MQTT_PORT 1883
#define MOUTH_ID "1"

// --- Configurazione PIN Sensore ---
#define TRIG_PIN 4
#define ECHO_PIN 5
#define TRIGGER_DISTANCE_CM 10 

// --- Configurazione PIN Pulsanti ---
#define CONFIRM_PIN 12
#define CORRECTION_PIN_PAPER 14
#define CORRECTION_PIN_PLASTIC 10
#define CORRECTION_PIN_GLASS 11

// --- Pulsante Annulla ---
#define CANCEL_PIN 47

// --- Configurazione Sensore Nastro ---
#define BELT_IR_SENSOR_PIN 13

// --- Configurazione Motore Nastro (L298N) ---
#define BELT_MOTOR_IN1_PIN 35  // Pin Direzione 1
#define BELT_MOTOR_IN2_PIN 36  // Pin Direzione 2
#define BELT_RUN_TIME 12000 

// --- Configurazione Debounce ---
#define DEBOUNCE_DELAY 50 // 50ms


// --- Configurazione Gate ---
#define SERVO_PIN 17
#define SERVO_POS_STOP 93       // Il punto di stop che hai trovato!
#define SERVO_VEL_APERTURA 40    // Velocità in una direzione (da 0 a 180)
#define SERVO_VEL_CHIUSURA 170      // Velocità nell'altra 
#define SERVO_TEMPO_MOVIMENTO 3100 // per aprirsi/chiudersi


// --- Servo Sorter

#define SORTER_PLASTIC_PIN 40
#define SORTER_PAPER_PIN 41
#define SORTER_GLASS_PIN 42

#define SORTER_POS_STOP 93
#define SORTER_VEL_AVANTI 180
#define SORTER_VEL_INDIETRO 0
#define SORTER_KICK_TIME 280 // 50ms