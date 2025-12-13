// include/NetworkManager.h
#pragma once
#include <PubSubClient.h>

// Dichiariamo queste variabili come "esterne"
// Saranno definite in NetworkManager.cpp, 
// ma main.cpp saprà che esistono.
extern PubSubClient client;
extern char MQTT_TOPIC_PROXIMITY[100];
extern char MQTT_TOPIC_CONFIRM[100];
extern char MQTT_TOPIC_CORRECTION[100];
extern char MQTT_TOPIC_CANCEL[100];
extern char MQTT_TOPIC_COMMAND[100];

void network_setup();
void network_loop(); // Questa funzione gestirà la riconnessione e il client.loop()