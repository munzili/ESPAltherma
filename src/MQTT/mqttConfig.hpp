#ifndef MQTT_CONFIG_H
#define MQTT_CONFIG_H

#include "Config/config.hpp"

#define MAX_MSG_SIZE 4096//max size of the json message sent in mqtt

extern char jsonbuff[MAX_MSG_SIZE];
extern bool SG_RELAY_ACTIVE_STATE;
extern bool SG_RELAY_INACTIVE_STATE;

void initMQTTConfig(Config *config);

#endif