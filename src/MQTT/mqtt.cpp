#include "mqtt.hpp"

WiFiClient espClient;
PubSubClient client(espClient);

String subscribeHeatingTopic = "";
String subscribeCoolingTopic = "";
String subscribeSGTopic = "";
String subscribePowerTopic = "";

String publishHeatingTopic = "";
String publishCoolingTopic = "";
String publishSGTopic = "";
String publishPowerTopic = "";
String publishAttrTopic = "";
String publishLWTTopic = "";

char jsonbuff[MAX_MSG_SIZE];
uint8_t SG_RELAY_ACTIVE_STATE;
uint8_t SG_RELAY_INACTIVE_STATE;

void initMQTT()
{
  client.setServer(config->MQTT_SERVER.c_str(), config->MQTT_PORT);
  client.setBufferSize(MAX_MSG_SIZE); // to support large json message
  client.setCallback(callback);

  createEmptyJSONBuffer();

  SG_RELAY_ACTIVE_STATE = (config->SG_RELAY_HIGH_TRIGGER == true) ? HIGH : LOW;
  SG_RELAY_INACTIVE_STATE = SG_RELAY_ACTIVE_STATE == HIGH ? LOW : HIGH;
}

void createEmptyJSONBuffer()
{
  if(config->MQTT_USE_JSONTABLE)
    strcpy(jsonbuff, "[{\0");
  else
    strcpy(jsonbuff, "{\0");
}

//Converts to string and add the value to the JSON message
void updateValues(ParameterDef *labelDef)
{
  bool alpha = false;
  for (size_t j = 0; j < strlen(labelDef->asString); j++)
  {
    char c = labelDef->asString[j];
    if (!isdigit(c) && c!='.')
    {
      alpha = true;
      break;
    }
  }

  if(config->MQTT_USE_ONETOPIC)
  {
    client.publish((config->MQTT_TOPIC_NAME + config->MQTT_ONETOPIC_NAME + labelDef->label).c_str(), labelDef->asString);
  }

  if(alpha)
  {
    snprintf(jsonbuff + strlen(jsonbuff), MAX_MSG_SIZE - strlen(jsonbuff), "\"%s\":\"%s\",", labelDef->label.c_str(), labelDef->asString);
  }
  else //number, no quotes
  {
    snprintf(jsonbuff + strlen(jsonbuff), MAX_MSG_SIZE - strlen(jsonbuff), "\"%s\":%s,", labelDef->label.c_str(), labelDef->asString);
  }
}

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

  client.publish(publishAttrTopic.c_str(), jsonbuff);

  createEmptyJSONBuffer();
}

void reconnect()
{
  //in case loopback as server is set, skip connecting (debug purpose)
  if(config->MQTT_SERVER.compareTo("127.0.0.1") == 0 || config->MQTT_SERVER.compareTo("localhost") == 0)
  {
    mqttSerial.print("Found loopback MQTT server, skiping connection...\n");
    return;
  }

  subscribeHeatingTopic = config->MQTT_TOPIC_NAME + MQTT_TOPIC_SUB_HEATING;
  subscribeCoolingTopic = config->MQTT_TOPIC_NAME + MQTT_TOPIC_SUB_COOLING;
  subscribeSGTopic      = config->MQTT_TOPIC_NAME + MQTT_TOPIC_SUB_SG;
  subscribePowerTopic   = config->MQTT_TOPIC_NAME + MQTT_TOPIC_SUB_POWER;

  publishHeatingTopic   = config->MQTT_TOPIC_NAME + MQTT_TOPIC_PUB_HEATING;
  publishCoolingTopic   = config->MQTT_TOPIC_NAME + MQTT_TOPIC_PUB_COOLING;
  publishSGTopic        = config->MQTT_TOPIC_NAME + MQTT_TOPIC_PUB_SG;
  publishPowerTopic     = config->MQTT_TOPIC_NAME + MQTT_TOPIC_PUB_POWER;
  publishAttrTopic      = config->MQTT_TOPIC_NAME + MQTT_TOPIC_PUB_ATTR;
  publishLWTTopic       = config->MQTT_TOPIC_NAME + MQTT_TOPIC_PUB_LWT;

  // Loop until we're reconnected
  int i = 0;
  while (!client.connected())
  {
    mqttSerial.print("Attempting MQTT connection...\n");

    String id = "ESPAltherma-";
    id += WiFi.macAddress().substring(6);

    mqttSerial.println(id);

    if (client.connect(id.c_str(), config->MQTT_USERNAME.c_str(), config->MQTT_PASSWORD.c_str(), publishLWTTopic.c_str(), 0, true, "Offline"))
    {
      mqttSerial.println("connected!");

      // TODO Update homeassistant config to publish correct informations
      client.publish("homeassistant/sensor/espAltherma/config", "{\"name\":\"AlthermaSensors\",\"stat_t\":\"~/STATESENS\",\"avty_t\":\"~/LWT\",\"pl_avail\":\"Online\",\"pl_not_avail\":\"Offline\",\"uniq_id\":\"espaltherma\",\"device\":{\"identifiers\":[\"ESPAltherma\"]}, \"~\":\"espaltherma\",\"json_attr_t\":\"~/ATTR\"}", true);
      client.publish(publishLWTTopic.c_str(), "Online", true);
      client.publish("homeassistant/switch/espAltherma/config", "{\"name\":\"Altherma Heating\",\"cmd_t\":\"~/STATE/HEATING\",\"stat_t\":\"~/STATE\",\"pl_off\":\"OFF\",\"pl_on\":\"ON\",\"~\":\"espaltherma\"}", true);
      client.publish("homeassistant/switch/espAltherma/config", "{\"name\":\"Altherma Cooling\",\"cmd_t\":\"~/STATE/COOLING\",\"stat_t\":\"~/STATE\",\"pl_off\":\"OFF\",\"pl_on\":\"ON\",\"~\":\"espaltherma\"}", true);
      client.publish("homeassistant/switch/espAltherma/config", "{\"name\":\"Altherma SmartGrid\",\"cmd_t\":\"~/STATE/SG\",\"stat_t\":\"~/STATE\",\"pl_off\":\"OFF\",\"pl_on\":\"ON\",\"~\":\"espaltherma\"}", true);

      // Subscribe
      // TODO Check if this subscribe could only be done once. will PubSubClient keep information after reconnect? Is the subscribtion added twice/multiple in this case?
      client.subscribe(subscribeHeatingTopic.c_str());
      client.subscribe(subscribeCoolingTopic.c_str());
      client.subscribe(subscribePowerTopic.c_str());

      mqttSerial.println("Subscribed to following topics:");
      mqttSerial.println(subscribeHeatingTopic);
      mqttSerial.println(subscribeCoolingTopic);
      mqttSerial.println(subscribePowerTopic);

      if(config->SG_ENABLED)
      {
        client.publish("homeassistant/sg/espAltherma/config", "{\"name\":\"AlthermaSmartGrid\",\"cmd_t\":\"~/set\",\"stat_t\":\"~/state\",\"~\":\"espaltherma/sg\"}", true);
        client.subscribe(subscribeSGTopic.c_str());

        mqttSerial.println(subscribeSGTopic);
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
    client.publish(publishHeatingTopic.c_str(), "OFF");
    mqttSerial.println("Heating turned OFF");
  }
  else if (payload[1] == 'N')
  { //turn on
    digitalWrite(config->PIN_HEATING, LOW);
    savePersistence();
    client.publish(publishHeatingTopic.c_str(), "ON");
    mqttSerial.println("Heating turned ON");
  }
  else
  {
    mqttSerial.printf("Unknown message: %s\n", payload);
  }
}

void callbackPower(byte *payload, unsigned int length)
{
  payload[length] = '\0';

  if (payload[0] == 'R')//R(eset/eboot)
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
    client.publish(publishCoolingTopic.c_str(), "OFF");
    mqttSerial.println("Cooling turned OFF");
  }
  else if (payload[1] == 'N')
  { //turn on
    digitalWrite(config->PIN_COOLING, LOW);
    savePersistence();
    client.publish(publishCoolingTopic.c_str(), "ON");
    mqttSerial.println("Cooling turned ON");
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
    client.publish(publishSGTopic.c_str(), "0");
    mqttSerial.println("Set SG mode to 0 - Normal operation");
  }
  else if (payload[0] == '1')
  {
    // Set SG 1 mode => SG1 = INACTIVE, SG2 = ACTIVE
    digitalWrite(config->PIN_SG1, SG_RELAY_INACTIVE_STATE);
    digitalWrite(config->PIN_SG2, SG_RELAY_ACTIVE_STATE);
    client.publish(publishSGTopic.c_str(), "1");
    mqttSerial.println("Set SG mode to 1 - Forced OFF");
  }
  else if (payload[0] == '2')
  {
    // Set SG 2 mode => SG1 = ACTIVE, SG2 = INACTIVE
    digitalWrite(config->PIN_SG1, SG_RELAY_ACTIVE_STATE);
    digitalWrite(config->PIN_SG2, SG_RELAY_INACTIVE_STATE);
    client.publish(publishSGTopic.c_str(), "2");
    mqttSerial.println("Set SG mode to 2 - Recommended ON");
  }
  else if (payload[0] == '3')
  {
    // Set SG 3 mode => SG1 = ACTIVE, SG2 = ACTIVE
    digitalWrite(config->PIN_SG1, SG_RELAY_ACTIVE_STATE);
    digitalWrite(config->PIN_SG2, SG_RELAY_ACTIVE_STATE);
    client.publish(publishSGTopic.c_str(), "3");
    mqttSerial.println("Set SG mode to 3 - Forced ON");
  }
  else
  {
    mqttSerial.printf("Unknown message: %s\n", payload);
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  char payloadText[length+1];
  for (int i=0;i<length;i++) {
    payloadText[i] = (char)payload[i];
  }
  payloadText[length] = '\0';

  mqttSerial.printf("Message arrived [%s] : %s\n", topic, payloadText);

  if (subscribeHeatingTopic == topic)
  {
    callbackHeating(payload, length);
  }
  else if (subscribeCoolingTopic == topic)
  {
    callbackCooling(payload, length);
  }
  else if (subscribePowerTopic == topic)
  {
    callbackPower(payload, length);
  }
  else if (config->SG_ENABLED && subscribeSGTopic == topic)
  {
    callbackSg(payload, length);
  }
  else
  {
    mqttSerial.printf("Unknown topic: %s\n", topic);
  }
}