#ifndef config_h
#define config_h
#include <stdint.h>
#include "labeldef.h"
#include "ArduinoJson.h"
#include <LittleFS.h>

#define MAX_MSG_SIZE 4096//max size of the json message sent in mqtt 

#define CONFIG_FILE "/config.json"
#define MODELS_CONFIG_SIZE 1024*10

typedef struct 
{
    bool configStored;
    bool startStandaloneWifi;
    char SSID[32];
    char SSID_PASSWORD[32];
    bool SSID_STATIC_IP;
    char SSID_IP[15];
    char SSID_SUBNET[15];
    char SSID_GATEWAY[15];
    char SSID_PRIMARY_DNS[15];
    char SSID_SECONDARY_DNS[15];
    char MQTT_SERVER[32];
    char MQTT_USERNAME[32];
    char MQTT_PASSWORD[32];
    bool MQTT_USE_JSONTABLE;
    bool MQTT_USE_ONETOPIC;
    char MQTT_ONETOPIC_NAME[32];
    uint16_t MQTT_PORT;
    uint32_t FREQUENCY;
    uint8_t PIN_RX;
    uint8_t PIN_TX;
    uint8_t PIN_THERM;
    bool SG_ENABLED;
    uint8_t PIN_SG1;
    uint8_t PIN_SG2;
    bool SG_RELAY_HIGH_TRIGGER;
    uint8_t MODEL;
    uint8_t LANGUAGE;
    size_t PARAMETERS_LENGTH;
    LabelDef** PARAMETERS;
} Config;

Config* config = nullptr;

void charPointerToArray(char* array, size_t length, const char* ptr)
{
    for(size_t i = 0; i < length; i++)
    {
        array[i] = ptr[i];
        
        if(ptr[i] == '\0')
            return;            
    }
}

void readConfig()
{
    config = new Config();
    
    if(!LittleFS.exists(CONFIG_FILE))
        return;

    File configFile = LittleFS.open(CONFIG_FILE, FILE_READ);
    DynamicJsonDocument configDoc(MODELS_CONFIG_SIZE);
    deserializeJson(configDoc, configFile.readString()); 
    configFile.close();    

    config->configStored = true;
    config->startStandaloneWifi = configDoc["startStandaloneWifi"].as<const bool>();
    charPointerToArray(config->SSID, 32, configDoc["SSID"].as<const char*>());
    charPointerToArray(config->SSID_PASSWORD, 32, configDoc["SSID_PASSWORD"].as<const char*>());
    config->SSID_STATIC_IP = configDoc["SSID_STATIC_IP"].as<const bool>();
    charPointerToArray(config->SSID_IP, 15, configDoc["SSID_IP"].as<const char*>());
    charPointerToArray(config->SSID_SUBNET, 15, configDoc["SSID_SUBNET"].as<const char*>());
    charPointerToArray(config->SSID_GATEWAY, 15, configDoc["SSID_GATEWAY"].as<const char*>());
    charPointerToArray(config->SSID_PRIMARY_DNS, 15, configDoc["SSID_PRIMARY_DNS"].as<const char*>());
    charPointerToArray(config->SSID_SECONDARY_DNS, 15, configDoc["SSID_SECONDARY_DNS"].as<const char*>());
    charPointerToArray(config->MQTT_SERVER, 32, configDoc["MQTT_SERVER"].as<const char*>());
    charPointerToArray(config->MQTT_USERNAME, 32, configDoc["MQTT_USERNAME"].as<const char*>());
    charPointerToArray(config->MQTT_PASSWORD, 32, configDoc["MQTT_PASSWORD"].as<const char*>());
    config->MQTT_USE_JSONTABLE = configDoc["MQTT_USE_JSONTABLE"].as<const bool>();
    config->MQTT_USE_ONETOPIC = configDoc["MQTT_USE_ONETOPIC"].as<const bool>();
    charPointerToArray(config->MQTT_ONETOPIC_NAME, 32, configDoc["MQTT_ONETOPIC_NAME"].as<const char*>());
    config->MQTT_PORT = configDoc["MQTT_PORT"].as<uint16_t>();
    config->FREQUENCY = configDoc["FREQUENCY"].as<uint32_t>();
    config->PIN_RX = configDoc["PIN_RX"].as<uint8_t>();
    config->PIN_TX = configDoc["PIN_TX"].as<uint8_t>();
    config->PIN_THERM = configDoc["PIN_THERM"].as<uint8_t>();
    config->SG_ENABLED = configDoc["SG_ENABLED"].as<const bool>();
    config->PIN_SG1 = configDoc["PIN_SG1"].as<uint8_t>();
    config->PIN_SG2 = configDoc["PIN_SG2"].as<uint8_t>();
    config->SG_RELAY_HIGH_TRIGGER = configDoc["SG_RELAY_HIGH_TRIGGER"].as<const bool>();
    config->MODEL = configDoc["MODEL"].as<uint8_t>();
    config->LANGUAGE = configDoc["LANGUAGE"].as<uint8_t>();

    JsonArray parameters = configDoc["PARAMETERS"].as<JsonArray>();
    config->PARAMETERS_LENGTH = parameters.size();
    config->PARAMETERS = new LabelDef*[config->PARAMETERS_LENGTH];

    for(size_t i = 0; i < config->PARAMETERS_LENGTH; i++)
    {
        JsonArray parameter = parameters[i];
        config->PARAMETERS[i] = new LabelDef(
            parameter[0].as<const int>(),
            parameter[1].as<const int>(),
            parameter[2].as<const int>(), 
            parameter[3].as<const int>(), 
            parameter[4].as<const int>(), 
            parameter[5].as<const char*>());
    }
}

void saveConfig()
{
    DynamicJsonDocument configDoc(MODELS_CONFIG_SIZE);
    configDoc["startStandaloneWifi"] = config->startStandaloneWifi;
    configDoc["SSID"] = config->SSID;
    configDoc["SSID_PASSWORD"] = config->SSID_PASSWORD;
    configDoc["SSID_STATIC_IP"] = config->SSID_STATIC_IP;
    configDoc["SSID_IP"] = config->SSID_IP;
    configDoc["SSID_SUBNET"] = config->SSID_SUBNET;
    configDoc["SSID_GATEWAY"] = config->SSID_GATEWAY;
    configDoc["SSID_PRIMARY_DNS"] = config->SSID_PRIMARY_DNS;
    configDoc["SSID_SECONDARY_DNS"] = config->SSID_SECONDARY_DNS;
    configDoc["MQTT_SERVER"] = config->MQTT_SERVER;
    configDoc["MQTT_USERNAME"] = config->MQTT_USERNAME;
    configDoc["MQTT_PASSWORD"] = config->MQTT_PASSWORD;
    configDoc["MQTT_USE_JSONTABLE"] = config->MQTT_USE_JSONTABLE;
    configDoc["MQTT_USE_ONETOPIC"] = config->MQTT_USE_ONETOPIC;
    configDoc["MQTT_ONETOPIC_NAME"] = config->MQTT_ONETOPIC_NAME;
    configDoc["MQTT_PORT"] = config->MQTT_PORT;
    configDoc["FREQUENCY"] = config->FREQUENCY;
    configDoc["PIN_RX"] = config->PIN_RX;
    configDoc["PIN_TX"] = config->PIN_TX;
    configDoc["PIN_THERM"] = config->PIN_THERM;
    configDoc["SG_ENABLED"] = config->SG_ENABLED;
    configDoc["PIN_SG1"] = config->PIN_SG1;
    configDoc["PIN_SG2"] = config->PIN_SG2;
    configDoc["SG_RELAY_HIGH_TRIGGER"] = config->SG_RELAY_HIGH_TRIGGER;
    configDoc["MODEL"] = config->MODEL;
    configDoc["LANGUAGE"] = config->LANGUAGE;
    
    JsonArray parameters = configDoc["PARAMETERS"].createNestedArray();

    for(size_t i = 0; i < config->PARAMETERS_LENGTH; i++)
    {
        JsonArray parameter = parameters.createNestedArray();
        parameter.add(config->PARAMETERS[i]->registryID);
        parameter.add(config->PARAMETERS[i]->offset);
        parameter.add(config->PARAMETERS[i]->convid);
        parameter.add(config->PARAMETERS[i]->dataSize);
        parameter.add(config->PARAMETERS[i]->dataType);
        parameter.add(config->PARAMETERS[i]->label);
    }
    
    File configFile = LittleFS.open(CONFIG_FILE, FILE_WRITE);
    serializeJson(configDoc, configFile);
    configFile.close();    
}

#endif