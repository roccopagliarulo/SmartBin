// include/NetworkManager.h
#pragma once
#include <PubSubClient.h>

// Declare these variables as "extern"
// They will be defined in NetworkManager.cpp,
// but main.cpp will know they exist.
extern PubSubClient client;
extern char MQTT_TOPIC_PROXIMITY[100];
extern char MQTT_TOPIC_CONFIRM[100];
extern char MQTT_TOPIC_CORRECTION[100];
extern char MQTT_TOPIC_CANCEL[100];
extern char MQTT_TOPIC_COMMAND[100];

void network_setup();
void network_loop(); // This function will handle reconnection and client.loop()