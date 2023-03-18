#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <Preferences.h>
#include "Config/config.hpp"
#include "MQTT/mqttSerial.hpp"

#define NAME_NAMESPACE "ESPAltherma"
#define NAME_INITED "Inited"
#define NAME_STATE_THERM "Therm"

extern Preferences preferences;

void resetPersistence();

void initPersistence();

void savePersistence(uint8_t state);

void readPersistence();

#endif