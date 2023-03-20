#include "persistence.hpp"

Preferences preferences;

void resetPersistence()
{
  preferences.clear();
}

void initPersistence()
{
  preferences.begin(NAME_NAMESPACE, false);
}

void savePersistence()
{
  preferences.putUChar(NAME_STATE_HEATING, digitalRead(config->PIN_HEATING));
  preferences.putUChar(NAME_STATE_COOLING, digitalRead(config->PIN_COOLING));
}

void readPersistence()
{
  bool inited = preferences.getBool(NAME_INITED);

  if (inited)
  {
    uint8_t heatingState = preferences.getUChar(NAME_STATE_HEATING);
    uint8_t coolingState = preferences.getUChar(NAME_STATE_COOLING);

    digitalWrite(config->PIN_HEATING, heatingState);
    digitalWrite(config->PIN_COOLING, coolingState);
    mqttSerial.printf("Restoring previous heating state: %s\n", (heatingState == HIGH) ? "Off":"On" );
    mqttSerial.printf("Restoring previous cooling state: %s\n", (coolingState == HIGH) ? "Off":"On" );
  }
  else
  {
    mqttSerial.printf("Persistence not initialized (%d). Initializing...\n", inited);
    preferences.putBool(NAME_INITED, true);

    digitalWrite(config->PIN_HEATING, HIGH);
    digitalWrite(config->PIN_COOLING, HIGH);

    savePersistence();
  }
}