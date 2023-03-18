#ifndef MQTT_H
#define MQTT_H

#include <PubSubClient.h>
#include "Config/config.h"
#include "mqttConfig.h"
#include "persistence.h"

#define MQTT_attr "espaltherma/ATTR"
#define MQTT_lwt "espaltherma/LWT"

#define EEPROM_CHK 1
#define EEPROM_STATE 0

extern PubSubClient client;

void sendValues();

void reconnect();

void callbackTherm(byte *payload, unsigned int length);

//Smartgrid callbacks
void callbackSg(byte *payload, unsigned int length);

void callback(char *topic, byte *payload, unsigned int length);

#endif