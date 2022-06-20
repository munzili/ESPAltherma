#ifdef ARDUINO_M5Stick_C_Plus
#include <M5StickCPlus.h>
#elif ARDUINO_M5Stick_C
#include <M5StickC.h>
#else
#include <Arduino.h>
#endif

#include <HardwareSerial.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

#include "config.h"
#include "mqttConfig.h"
#include "mqttSerial.h"
#include "converters.h"
#include "comm.h"
#include "mqtt.h"
#include "webui.h"

size_t registryBufferSize;
RegistryBuffer *registryBuffers; //Holds the registries to query and the last returned data

bool busy = false;
bool configWifiEnabled;

#if defined(ARDUINO_M5Stick_C) || defined(ARDUINO_M5Stick_C_Plus)
long LCDTimeout = 40000;//Keep screen ON for 40s then turn off. ButtonA will turn it On again.
#endif

bool contains(uint8_t array[], size_t size, uint8_t value)
{
  for (size_t i = 0; i < size; i++)
  {
    if (array[i] == value)
      return true;
  }
  return false;
}

//Converts to string and add the value to the JSON message
void updateValues(LabelDef *labelDef)
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
    char *topicBuff = config->MQTT_ONETOPIC_NAME;
    strcat(topicBuff, labelDef->label);
    client.publish(config->MQTT_ONETOPIC_NAME, labelDef->asString);
  }
  
  if (alpha)
  {      
    snprintf(jsonbuff + strlen(jsonbuff), MAX_MSG_SIZE - strlen(jsonbuff), "\"%s\":\"%s\",", labelDef->label, labelDef->asString);
  }
  else //number, no quotes
  {
    snprintf(jsonbuff + strlen(jsonbuff), MAX_MSG_SIZE - strlen(jsonbuff), "\"%s\":%s,", labelDef->label, labelDef->asString);
  }
}

uint16_t loopcount =0;

void extraLoop()
{
  client.loop();
  ArduinoOTA.handle();
  while (busy)
  { //Stop processing during OTA
    ArduinoOTA.handle();
  }

#if defined(ARDUINO_M5Stick_C) || defined(ARDUINO_M5Stick_C_Plus)
  if (M5.BtnA.wasPressed()){//Turn back ON screen
    M5.Axp.ScreenBreath(12);
    LCDTimeout = millis() + 30000;
  }else if (LCDTimeout < millis()){//Turn screen off.
    M5.Axp.ScreenBreath(0);
  }
  M5.update();
#endif
}

void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  mqttSerial.printf("Connecting to %s\n", config->SSID);

  if(config->SSID_STATIC_IP)
  {
    IPAddress local_IP;
    IPAddress subnet;
    IPAddress gateway;
    IPAddress primaryDNS;
    IPAddress secondaryDNS;

    local_IP.fromString(config->SSID_IP);
    subnet.fromString(config->SSID_SUBNET);
    gateway.fromString(config->SSID_GATEWAY);

    if(config->SSID_PRIMARY_DNS != "")
    {
      primaryDNS.fromString(config->SSID_PRIMARY_DNS);
    }

    if(config->SSID_SECONDARY_DNS != "")
    {
      secondaryDNS.fromString(config->SSID_SECONDARY_DNS);
    }

    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
        mqttSerial.println("Failed to set static ip!");
    }
  }

  WiFi.begin(config->SSID, config->SSID_PASSWORD);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    if (i++ == 100)
    {
      esp_restart();
    }
  }
  mqttSerial.printf("Connected. IP Address: %s\n", WiFi.localIP().toString().c_str());
}

void initRegistries()
{
  //getting the list of registries to query from the selected values  
  registryBufferSize = 0;  
  uint8_t* tempRegistryIDs = new uint8_t[config->PARAMETERS_LENGTH];

  size_t i;
  for (i = 0; i < config->PARAMETERS_LENGTH; i++)
  {            
    auto &&label = *config->PARAMETERS[i];

    if (!contains(tempRegistryIDs, sizeof(tempRegistryIDs), label.registryID))
    {
      mqttSerial.printf("Adding registry 0x%2x to be queried.\n", label.registryID);
      tempRegistryIDs[registryBufferSize++] = label.registryID;
    }
  }

  registryBuffers = new RegistryBuffer[registryBufferSize];

  for(i = 0; i < registryBufferSize; i++)
  {
    registryBuffers[i].RegistryID = tempRegistryIDs[i];
  }

  delete[] tempRegistryIDs;

  if (registryBufferSize == 0)
  {
    mqttSerial.printf("ERROR - No parameter definition selected in the config. Stopping.\n");
    while (true)
    {
      extraLoop();
    }
  }
}

void setupScreen(){
#if defined(ARDUINO_M5Stick_C) || defined(ARDUINO_M5Stick_C_Plus)
  M5.begin();
  M5.Axp.EnableCoulombcounter();
  M5.Lcd.setRotation(1);
  M5.Axp.ScreenBreath(12);
  M5.Lcd.fillScreen(TFT_WHITE);
  M5.Lcd.setFreeFont(&FreeSansBold12pt7b);
  m5.Lcd.setTextDatum(MC_DATUM);
  int xpos = M5.Lcd.width() / 2; // Half the screen width
  int ypos = M5.Lcd.height() / 2; // Half the screen width
  M5.Lcd.setTextColor(TFT_DARKGREY);
  M5.Lcd.drawString("ESPAltherma", xpos, ypos, 1);
  delay(2000);
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextFont(1);
  M5.Lcd.setTextColor(TFT_GREEN);
#endif
}

void setup()
{
  Serial.begin(115200);
  
  if(!LittleFS.begin(true)) 
  {
      Serial.println("An Error has occurred while mounting LittleFS");
      return;
  }
  
  EEPROM.begin(16);

  readConfig();

  if(config->startStandaloneWifi || !config->configStored)
  {
    IPAddress local_ip(192, 168, 1, 1); 
    IPAddress gateway(192, 168, 1, 1); 
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAP("ESPAltherma-Config-WiFi");   
    WiFi.softAPConfig(local_ip, gateway, subnet);
    WiFi.setHostname("ESPAltherma");    
  }
  
  WebUI_Init();

  initMQTTConfig(config);

  setupScreen();

  if(!config->configStored)
  {
    mqttSerial.print("No config found, skip setup...");
    return;
  }

  SerialX10A.begin(9600, SERIAL_8E1, config->PIN_RX, config->PIN_TX);
  pinMode(config->PIN_THERM, OUTPUT);
  digitalWrite(config->PIN_THERM, HIGH);

  if(config->SG_ENABLED)
  {
    //Smartgrid pins - Set first to the inactive state, before configuring as outputs (avoid false triggering when initializing)
    digitalWrite(config->PIN_SG1, SG_RELAY_INACTIVE_STATE);
    digitalWrite(config->PIN_SG2, SG_RELAY_INACTIVE_STATE);
    pinMode(config->PIN_SG1, OUTPUT);
    pinMode(config->PIN_SG2, OUTPUT);
  }

#ifdef ARDUINO_M5Stick_C_Plus
  gpio_pulldown_dis(GPIO_NUM_25);
  gpio_pullup_dis(GPIO_NUM_25);
#endif

  readEEPROM();//Restore previous state

  if(!config->startStandaloneWifi)
  {
    mqttSerial.print("Setting up wifi...");
    setup_wifi();
  }

  ArduinoOTA.setHostname("ESPAltherma");
  ArduinoOTA.onStart([]() {
    busy = true;
  });

  ArduinoOTA.onError([](ota_error_t error) {
    mqttSerial.print("Error on OTA - restarting");
    esp_restart();
  });
  ArduinoOTA.begin();

  client.setServer(config->MQTT_SERVER, config->MQTT_PORT);
  client.setBufferSize(MAX_MSG_SIZE); //to support large json message
  client.setCallback(callback);
  client.setServer(config->MQTT_SERVER, config->MQTT_PORT);
  mqttSerial.print("Connecting to MQTT server...");
  mqttSerial.begin(&client, "espaltherma/log");
  reconnect();
  mqttSerial.println("OK!");

  initRegistries();
  mqttSerial.print("ESPAltherma started!");
}

void waitLoop(uint ms){
      unsigned long start = millis();
      while (millis() < start + ms) //wait .5sec between registries
      {
        extraLoop();
      }
}

void loop()
{
  if(!config->configStored)
  {
    extraLoop();
    return;
  }

  if (!client.connected())
  { //(re)connect to MQTT if needed
    reconnect();
  }

  //Querying all registries and store results
  for (size_t i = 0; i < registryBufferSize; i++)
  {
    int tries = 0;
    while (!queryRegistry(&registryBuffers[i]) && tries++ < 3)
    {
      mqttSerial.println("Retrying...");
      waitLoop(1000);
    }
  }

  for (int i = 0; i < config->PARAMETERS_LENGTH; i++)
  {            
    auto &&label = *config->PARAMETERS[i];

    for (size_t j = 0; j < registryBufferSize; j++)
    {
      if(label.registryID == registryBuffers[j].RegistryID)
      {
        char *input = registryBuffers[j].Buffer;
        input += label.offset + 3;

        converter.convert(&label, input); // convert buffer result of label offset to correct/usabel value

        updateValues(&label);       //send them in mqtt
        waitLoop(500);//wait .5sec between registries
        break;
      }
    }
  }  
  
  sendValues();//Send the full json message
  mqttSerial.printf("Done. Waiting %d sec...\n", config->FREQUENCY / 1000);
  waitLoop(config->FREQUENCY);
}
