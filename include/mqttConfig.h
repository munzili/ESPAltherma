#ifndef MQTT_CONFIG_H
#define MQTT_CONFIG_H
#include "config.h"

#define MAX_MSG_SIZE 4096//max size of the json message sent in mqtt 

char jsonbuff[MAX_MSG_SIZE];
bool SG_RELAY_ACTIVE_STATE;
bool SG_RELAY_INACTIVE_STATE;

void initMQTTConfig(Config *config)
{
  if(config->MQTT_USE_JSONTABLE)
    char jsonbuff[MAX_MSG_SIZE] = "[{\0";
  else
    char jsonbuff[MAX_MSG_SIZE] = "{\0";

  SG_RELAY_ACTIVE_STATE = config->SG_RELAY_HIGH_TRIGGER == true;
  SG_RELAY_INACTIVE_STATE != SG_RELAY_ACTIVE_STATE;
}
#endif