#ifndef PERSISTENCE_H
#define PERSISTENCE_H
#include <Preferences.h>
#include "config.h"
#include "mqttSerial.h"

#define NAME_NAMESPACE "ESPAltherma"
#define NAME_INITED "Inited"
#define NAME_STATE_THERM "Therm"

Preferences preferences;

void initPersistence()
{
  preferences.begin(NAME_NAMESPACE, false); 
}

void savePersistence(uint8_t state){
  preferences.putUChar(NAME_STATE_THERM, state);
}

void readPersistence(){
  bool inited = preferences.getBool(NAME_INITED);

  if (inited)
  {
    uint8_t state = preferences.getUChar(NAME_STATE_THERM);
    digitalWrite(config->PIN_THERM, state);
    mqttSerial.printf("Restoring previous state: %s", (state == HIGH) ? "Off":"On" );
  }
  else
  {
    mqttSerial.printf("EEPROM not initialized (%d). Initializing...", inited);
    preferences.putBool(NAME_INITED, true);
    preferences.putUChar(NAME_STATE_THERM, HIGH);
    digitalWrite(config->PIN_THERM, HIGH);
  }
}

#endif