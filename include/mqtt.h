#ifndef mqtt_h
#define mqtt_h

#include <PubSubClient.h>
#include "eepromState.h"
#define MQTT_attr "espaltherma/ATTR"
#define MQTT_lwt "espaltherma/LWT"

#ifdef JSONTABLE
char jsonbuff[MAX_MSG_SIZE] = "[{\0";
#else
char jsonbuff[MAX_MSG_SIZE] = "{\0";
#endif

WiFiClient espClient;
PubSubClient client(espClient);

void sendValues()
{
  Serial.printf("Sending values in MQTT.\n");
#ifdef ARDUINO_M5Stick_C
  //Add M5 APX values
  snprintf(jsonbuff + strlen(jsonbuff),MAX_MSG_SIZE - strlen(jsonbuff) , "\"%s\":\"%.3gV\",\"%s\":\"%gmA\",", "M5VIN", M5.Axp.GetVinVoltage(),"M5AmpIn", M5.Axp.GetVinCurrent());
  snprintf(jsonbuff + strlen(jsonbuff),MAX_MSG_SIZE - strlen(jsonbuff) , "\"%s\":\"%.3gV\",\"%s\":\"%gmA\",", "M5BatV", M5.Axp.GetBatVoltage(),"M5BatCur", M5.Axp.GetBatCurrent());
  snprintf(jsonbuff + strlen(jsonbuff),MAX_MSG_SIZE - strlen(jsonbuff) , "\"%s\":\"%.3gmW\",", "M5BatPwr", M5.Axp.GetBatPower());
#endif  
  snprintf(jsonbuff + strlen(jsonbuff),MAX_MSG_SIZE - strlen(jsonbuff) , "\"%s\":\"%ddBm\",", "WifiRSSI", WiFi.RSSI());

  jsonbuff[strlen(jsonbuff) - 1] = '}';
#ifdef JSONTABLE
  strcat(jsonbuff,"]");
#endif
  client.publish(MQTT_attr, jsonbuff);
#ifdef JSONTABLE
  strcpy(jsonbuff, "[{\0");
#else
  strcpy(jsonbuff, "{\0");
#endif
}

void reconnect()
{
  // Loop until we're reconnected
  int i = 0;
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");

    if (client.connect("ESPAltherma-dev", MQTT_USERNAME, MQTT_PASSWORD, MQTT_lwt, 0, true, "Offline"))
    {
      Serial.println("connected!");
      client.publish("homeassistant/sensor/espAltherma/config", "{\"name\":\"AlthermaSensors\",\"stat_t\":\"~/STATESENS\",\"avty_t\":\"~/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"espaltherma\",\"device\":{\"identifiers\":[\"ESPAltherma\"]}, \"~\":\"espaltherma\",\"json_attr_t\":\"~/ATTR\"}", true);
      client.publish(MQTT_lwt, "Online", true);
      client.publish("homeassistant/switch/espAltherma/config", "{\"name\":\"Altherma\",\"cmd_t\":\"~/POWER\",\"stat_t\":\"~/STATE\",\"pl_off\":\"OFF\",\"pl_on\":\"ON\",\"~\":\"espaltherma\"}", true);
 
      // Subscribe
      client.subscribe("espaltherma/POWER");
#ifdef PIN_SG1
      client.publish("homeassistant/sg/espAltherma/config", "{\"name\":\"AlthermaSmartGrid\",\"cmd_t\":\"~/set\",\"stat_t\":\"~/state\",\"~\":\"espaltherma/sg\"}", true);
      client.subscribe("espaltherma/sg/set");
#endif
    }
    else
    {
      Serial.printf("failed, rc=%d, try again in 5 seconds", client.state());
      unsigned long start = millis();
      while (millis() < start + 5000)
      {
        ArduinoOTA.handle();
      }

      if (i++ == 100)
        Serial.printf("Tried for 500 sec, rebooting now.");
        esp_restart();
    }
  }
}

void callbackReboot()
{  
    mqttSerial.println("Rebooting");
    delay(100);
    esp_restart();
}

#ifdef PIN_RT_HEATING
void callbackHeating(byte *payload, unsigned int length)
{  
  payload[length] = '\0';
  
  // Is it ON or OFF?
  // Ok I'm not super proud of this, but it works :p 
  if (payload[1] == 'F')
  { //turn off
    digitalWrite(PIN_RT_HEATING, HIGH);
    saveEEPROM(EEPROM_HEATING, HIGH);
    client.publish("espaltherma/state/heating", "OFF");
    mqttSerial.println("Turned heating OFF");
  }
  else if (payload[1] == 'N')
  { //turn on
    digitalWrite(PIN_RT_HEATING, LOW);
    saveEEPROM(EEPROM_HEATING, LOW);
    client.publish("espaltherma/state/heating", "ON");
    mqttSerial.println("Turned heating ON");
  }
  else
  {
    Serial.printf("Unknown message: %s\n", payload);
  }
}
#endif

#ifdef PIN_RT_COOLING
void callbackCooling(byte *payload, unsigned int length)
{  
  payload[length] = '\0';
  
  // Is it ON or OFF?
  // Ok I'm not super proud of this, but it works :p 
  if (payload[1] == 'F')
  { //turn off
    digitalWrite(PIN_RT_COOLING, HIGH);
    saveEEPROM(EEPROM_COOLING, HIGH);
    client.publish("espaltherma/state/cooling", "OFF");
    mqttSerial.println("Turned cooling OFF");
  }
  else if (payload[1] == 'N')
  { //turn on
    digitalWrite(PIN_RT_COOLING, LOW);
    saveEEPROM(EEPROM_COOLING, LOW);
    client.publish("espaltherma/state/cooling", "ON");
    mqttSerial.println("Turned cooling ON");
  }
  else
  {
    Serial.printf("Unknown message: %s\n", payload);
  }
}
#endif

#ifdef PIN_SG1
//Smartgrid callbacks
void callbackSg(byte *payload, unsigned int length)
{
  payload[length] = '\0';
  
  if (payload[0] == '0')
  {
    // Set SG 0 mode => SG1 = INACTIVE, SG2 = INACTIVE
    digitalWrite(PIN_SG1, SG_RELAY_INACTIVE_STATE);
    digitalWrite(PIN_SG2, SG_RELAY_INACTIVE_STATE);
    saveEEPROM(EEPROM_SG1, SG_RELAY_INACTIVE_STATE);
    saveEEPROM(EEPROM_SG2, SG_RELAY_INACTIVE_STATE);
    client.publish("espaltherma/sg/state", "0");
    Serial.println("Set SG mode to 0 - Normal operation");
  }
  else if (payload[0] == '1')
  {
    // Set SG 1 mode => SG1 = INACTIVE, SG2 = ACTIVE
    digitalWrite(PIN_SG1, SG_RELAY_INACTIVE_STATE);
    digitalWrite(PIN_SG2, SG_RELAY_ACTIVE_STATE);
    saveEEPROM(EEPROM_SG1, SG_RELAY_INACTIVE_STATE);
    saveEEPROM(EEPROM_SG2, SG_RELAY_ACTIVE_STATE);
    client.publish("espaltherma/sg/state", "1");
    Serial.println("Set SG mode to 1 - Forced OFF");
  }
  else if (payload[0] == '2')
  {
    // Set SG 2 mode => SG1 = ACTIVE, SG2 = INACTIVE
    digitalWrite(PIN_SG1, SG_RELAY_ACTIVE_STATE);
    digitalWrite(PIN_SG2, SG_RELAY_INACTIVE_STATE);
    client.publish("espaltherma/sg/state", "2");
    Serial.println("Set SG mode to 2 - Recommended ON");
  }
  else if (payload[0] == '3')
  {
    // Set SG 3 mode => SG1 = ACTIVE, SG2 = ACTIVE
    digitalWrite(PIN_SG1, SG_RELAY_ACTIVE_STATE);
    digitalWrite(PIN_SG2, SG_RELAY_ACTIVE_STATE);
    saveEEPROM(EEPROM_SG1, SG_RELAY_ACTIVE_STATE);
    saveEEPROM(EEPROM_SG2, SG_RELAY_ACTIVE_STATE);
    client.publish("espaltherma/sg/state", "3");
    Serial.println("Set SG mode to 3 - Forced ON");
  }
  else
  {
    Serial.printf("Unknown message: %s\n", payload);
  }
}
#endif

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.printf("Message arrived [%s] : %s\n", topic, payload);

#ifdef PIN_RT_HEATING
  if (strcmp(topic, "espaltherma/set/heating") == 0)
  {
    callbackHeating(payload, length);
  }
#else
  if(false){}
#endif 

#ifdef PIN_RT_COOLING
  else if (strcmp(topic, "espaltherma/set/cooling") == 0)
  {
    callbackCooling(payload, length);
  }
#endif

#ifdef PIN_SG1
  else if (strcmp(topic, "espaltherma/set/sg") == 0)
  {
    callbackSg(payload, length);
  }
#endif

  else if (strcmp(topic, "espaltherma/reboot") == 0)
  {
    callbackReboot();
  }

  else
  {
    Serial.printf("Unknown topic: %s\n", topic);
  }
}
#endif