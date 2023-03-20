#include "mqtt.hpp"

WiFiClient espClient;
PubSubClient client(espClient);

void sendValues()
{
  mqttSerial.printf("Sending values in MQTT.\n");
#ifdef ARDUINO_M5Stick_C
  //Add M5 APX values
  snprintf(jsonbuff + strlen(jsonbuff),MAX_MSG_SIZE - strlen(jsonbuff) , "\"%s\":\"%.3gV\",\"%s\":\"%gmA\",", "M5VIN", M5.Axp.GetVinVoltage(),"M5AmpIn", M5.Axp.GetVinCurrent());
  snprintf(jsonbuff + strlen(jsonbuff),MAX_MSG_SIZE - strlen(jsonbuff) , "\"%s\":\"%.3gV\",\"%s\":\"%gmA\",", "M5BatV", M5.Axp.GetBatVoltage(),"M5BatCur", M5.Axp.GetBatCurrent());
  snprintf(jsonbuff + strlen(jsonbuff),MAX_MSG_SIZE - strlen(jsonbuff) , "\"%s\":\"%.3gmW\",", "M5BatPwr", M5.Axp.GetBatPower());
#endif
  snprintf(jsonbuff + strlen(jsonbuff),MAX_MSG_SIZE - strlen(jsonbuff) , "\"%s\":\"%ddBm\",", "WifiRSSI", WiFi.RSSI());

  jsonbuff[strlen(jsonbuff) - 1] = '}';

  if(config->MQTT_USE_JSONTABLE)
    strcat(jsonbuff,"]");

  client.publish(MQTT_attr, jsonbuff);

  if(config->MQTT_USE_JSONTABLE)
    strcpy(jsonbuff, "[{\0");
  else
    strcpy(jsonbuff, "{\0");
}

void reconnect()
{
  //in case loopback as server is set, skip connecting (debug purpose)
  if(config->MQTT_SERVER.compareTo("127.0.0.1") == 0 || config->MQTT_SERVER.compareTo( "localhost") == 0)
  {
    mqttSerial.print("Found loopback MQTT server, skiping connection...\n");
    return;
  }

  // Loop until we're reconnected
  int i = 0;
  while (!client.connected())
  {
    mqttSerial.print("Attempting MQTT connection...\n");

    if (client.connect("ESPAltherma-dev", config->MQTT_USERNAME.c_str(), config->MQTT_PASSWORD.c_str(), MQTT_lwt, 0, true, "Offline"))
    {
      mqttSerial.println("connected!");
      client.publish("homeassistant/sensor/espAltherma/config", "{\"name\":\"AlthermaSensors\",\"stat_t\":\"~/STATESENS\",\"avty_t\":\"~/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"espaltherma\",\"device\":{\"identifiers\":[\"ESPAltherma\"]}, \"~\":\"espaltherma\",\"json_attr_t\":\"~/ATTR\"}", true);
      client.publish(MQTT_lwt, "Online", true);
      client.publish("homeassistant/switch/espAltherma/config", "{\"name\":\"Altherma Heating\",\"cmd_t\":\"~/STATE/HEATING\",\"stat_t\":\"~/STATE\",\"pl_off\":\"OFF\",\"pl_on\":\"ON\",\"~\":\"espaltherma\"}", true);
      client.publish("homeassistant/switch/espAltherma/config", "{\"name\":\"Altherma Cooling\",\"cmd_t\":\"~/STATE/COOLING\",\"stat_t\":\"~/STATE\",\"pl_off\":\"OFF\",\"pl_on\":\"ON\",\"~\":\"espaltherma\"}", true);
      client.publish("homeassistant/switch/espAltherma/config", "{\"name\":\"Altherma SmartGrid\",\"cmd_t\":\"~/STATE/SG\",\"stat_t\":\"~/STATE\",\"pl_off\":\"OFF\",\"pl_on\":\"ON\",\"~\":\"espaltherma\"}", true);

      // Subscribe
      client.subscribe("espaltherma/SET/HEATING");
      client.subscribe("espaltherma/SET/COOLING");

      if(config->SG_ENABLED)
      {
        client.publish("homeassistant/sg/espAltherma/config", "{\"name\":\"AlthermaSmartGrid\",\"cmd_t\":\"~/set\",\"stat_t\":\"~/state\",\"~\":\"espaltherma/sg\"}", true);
        client.subscribe("espaltherma/SET/SG");
      }
    }
    else
    {
      mqttSerial.printf("failed, rc=%d, try again in 5 seconds", client.state());
      unsigned long start = millis();
      while (millis() < start + 5000) { }

      if (i++ == 100)
      {
        mqttSerial.printf("Tried for 500 sec, rebooting now.");
        esp_restart();
      }
    }
  }
}

void callbackHeating(byte *payload, unsigned int length)
{
  payload[length] = '\0';

  // Is it ON or OFF?
  // Ok I'm not super proud of this, but it works :p
  if (payload[1] == 'F')
  { //turn off
    digitalWrite(config->PIN_HEATING, HIGH);
    savePersistence();
    client.publish("espaltherma/STATE/HEATING", "OFF");
    mqttSerial.println("Turned OFF");
  }
  else if (payload[1] == 'N')
  { //turn on
    digitalWrite(config->PIN_HEATING, LOW);
    savePersistence();
    client.publish("espaltherma/STATE/HEATING", "ON");
    mqttSerial.println("Turned ON");
  }
  else if (payload[0] == 'R')//R(eset/eboot)
  {
    mqttSerial.println("Rebooting");
    delay(100);
    esp_restart();
  }
  else
  {
    mqttSerial.printf("Unknown message: %s\n", payload);
  }
}

void callbackCooling(byte *payload, unsigned int length)
{
  payload[length] = '\0';

  // Is it ON or OFF?
  // Ok I'm not super proud of this, but it works :p
  if (payload[1] == 'F')
  { //turn off
    digitalWrite(config->PIN_COOLING, HIGH);
    savePersistence();
    client.publish("espaltherma/STATE/COOLING", "OFF");
    mqttSerial.println("Turned OFF");
  }
  else if (payload[1] == 'N')
  { //turn on
    digitalWrite(config->PIN_COOLING, LOW);
    savePersistence();
    client.publish("espaltherma/STATE/COOLING", "ON");
    mqttSerial.println("Turned ON");
  }
  else if (payload[0] == 'R')//R(eset/eboot)
  {
    mqttSerial.println("Rebooting");
    delay(100);
    esp_restart();
  }
  else
  {
    mqttSerial.printf("Unknown message: %s\n", payload);
  }
}

//Smartgrid callbacks
void callbackSg(byte *payload, unsigned int length)
{
  payload[length] = '\0';

  if (payload[0] == '0')
  {
    // Set SG 0 mode => SG1 = INACTIVE, SG2 = INACTIVE
    digitalWrite(config->PIN_SG1, SG_RELAY_INACTIVE_STATE);
    digitalWrite(config->PIN_SG2, SG_RELAY_INACTIVE_STATE);
    client.publish("espaltherma/STATE/SG", "0");
    mqttSerial.println("Set SG mode to 0 - Normal operation");
  }
  else if (payload[0] == '1')
  {
    // Set SG 1 mode => SG1 = INACTIVE, SG2 = ACTIVE
    digitalWrite(config->PIN_SG1, SG_RELAY_INACTIVE_STATE);
    digitalWrite(config->PIN_SG2, SG_RELAY_ACTIVE_STATE);
    client.publish("espaltherma/STATE/SG", "1");
    mqttSerial.println("Set SG mode to 1 - Forced OFF");
  }
  else if (payload[0] == '2')
  {
    // Set SG 2 mode => SG1 = ACTIVE, SG2 = INACTIVE
    digitalWrite(config->PIN_SG1, SG_RELAY_ACTIVE_STATE);
    digitalWrite(config->PIN_SG2, SG_RELAY_INACTIVE_STATE);
    client.publish("espaltherma/STATE/SG", "2");
    mqttSerial.println("Set SG mode to 2 - Recommended ON");
  }
  else if (payload[0] == '3')
  {
    // Set SG 3 mode => SG1 = ACTIVE, SG2 = ACTIVE
    digitalWrite(config->PIN_SG1, SG_RELAY_ACTIVE_STATE);
    digitalWrite(config->PIN_SG2, SG_RELAY_ACTIVE_STATE);
    client.publish("espaltherma/STATE/SG", "3");
    mqttSerial.println("Set SG mode to 3 - Forced ON");
  }
  else
  {
    mqttSerial.printf("Unknown message: %s\n", payload);
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  mqttSerial.printf("Message arrived [%s] : %s\n", topic, payload);

  if (strcmp(topic, "espaltherma/SET/HEATING") == 0)
  {
    callbackHeating(payload, length);
  }
  else if (strcmp(topic, "espaltherma/SET/COOLING") == 0)
  {
    callbackCooling(payload, length);
  }
  else if (config->SG_ENABLED && strcmp(topic, "espaltherma/SET/SG") == 0)
  {
    callbackSg(payload, length);
  }
  else
  {
    mqttSerial.printf("Unknown topic: %s\n", topic);
  }
}