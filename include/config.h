#ifndef config_h
#define config_h
#include <stdint.h>
#include "labeldef.h"
#include "ArduinoJson.h"
#include <LittleFS.h>

#define MAX_MSG_SIZE 4096//max size of the json message sent in mqtt 

#define CONFIG_FILE "/config.json"
#define MODELS_CONFIG_SIZE 1024*10

struct Config
{
    bool configStored;
    bool startStandaloneWifi;
    char* SSID;
    char* SSID_PASSWORD;
    bool SSID_STATIC_IP;
    char* SSID_IP;
    char* SSID_SUBNET;
    char* SSID_GATEWAY;
    char* SSID_PRIMARY_DNS;
    char* SSID_SECONDARY_DNS;
    char* MQTT_SERVER;
    char* MQTT_USERNAME;
    char* MQTT_PASSWORD;
    bool MQTT_USE_JSONTABLE;
    bool MQTT_USE_ONETOPIC;
    char* MQTT_ONETOPIC_NAME;
    uint16_t MQTT_PORT;
    uint32_t FREQUENCY;
    uint8_t PIN_RX;
    uint8_t PIN_TX;
    uint8_t PIN_THERM;
    bool SG_ENABLED;
    uint8_t PIN_SG1;
    uint8_t PIN_SG2;
    bool SG_RELAY_HIGH_TRIGGER;
    uint8_t PIN_ENABLE_CONFIG;
    size_t PARAMETERS_LENGTH;
    LabelDef** PARAMETERS;

    ~Config()
    {
        if(SSID) delete[] SSID;
        if(SSID_PASSWORD) delete[] SSID_PASSWORD;
        if(SSID_IP) delete[] SSID_IP;
        if(SSID_SUBNET) delete[] SSID_SUBNET;
        if(SSID_GATEWAY) delete[] SSID_GATEWAY;
        if(SSID_PRIMARY_DNS) delete[] SSID_PRIMARY_DNS;
        if(SSID_SECONDARY_DNS) delete[] SSID_SECONDARY_DNS;
        if(MQTT_SERVER) delete[] MQTT_SERVER;
        if(MQTT_USERNAME) delete[] MQTT_USERNAME;
        if(MQTT_PASSWORD) delete[] MQTT_PASSWORD;
        if(MQTT_ONETOPIC_NAME) delete[] MQTT_ONETOPIC_NAME;

        if(PARAMETERS_LENGTH)
        {
            for (size_t i = 0; i < PARAMETERS_LENGTH; i++)
            {
                delete PARAMETERS[i];
            }
            delete[] PARAMETERS;
        }
    }
};

Config* config = nullptr;

void createArray(char* array, const char* str)
{
    array = new char[strlen(str)+1];
    strcpy(array, str);
}

void readConfig()
{
    if(config != nullptr)
        delete config;
        
    config = new Config();
    
    if(!LittleFS.exists(CONFIG_FILE))    
        return;    

    File configFile = LittleFS.open(CONFIG_FILE, FILE_READ);
    DynamicJsonDocument configDoc(MODELS_CONFIG_SIZE);
    deserializeJson(configDoc, configFile.readString()); 
    configFile.close();    

    config->configStored = true;
    config->startStandaloneWifi = configDoc["startStandaloneWifi"].as<const bool>();
    createArray(config->SSID, configDoc["SSID"].as<const char*>());
    createArray(config->SSID_PASSWORD, configDoc["SSID_PASSWORD"].as<const char*>());
    config->SSID_STATIC_IP = configDoc["SSID_STATIC_IP"].as<const bool>();
    if(config->SSID_STATIC_IP)
    {
        createArray(config->SSID_IP, configDoc["SSID_IP"].as<const char*>());
        createArray(config->SSID_SUBNET, configDoc["SSID_SUBNET"].as<const char*>());
        createArray(config->SSID_GATEWAY, configDoc["SSID_GATEWAY"].as<const char*>());
        createArray(config->SSID_PRIMARY_DNS, configDoc["SSID_PRIMARY_DNS"].as<const char*>());
        createArray(config->SSID_SECONDARY_DNS, configDoc["SSID_SECONDARY_DNS"].as<const char*>());
    }
    createArray(config->MQTT_SERVER, configDoc["MQTT_SERVER"].as<const char*>());
    createArray(config->MQTT_USERNAME, configDoc["MQTT_USERNAME"].as<const char*>());
    createArray(config->MQTT_PASSWORD, configDoc["MQTT_PASSWORD"].as<const char*>());
    config->MQTT_USE_JSONTABLE = configDoc["MQTT_USE_JSONTABLE"].as<const bool>();
    config->MQTT_USE_ONETOPIC = configDoc["MQTT_USE_ONETOPIC"].as<const bool>();
    if(config->MQTT_USE_ONETOPIC)
    {
        createArray(config->MQTT_ONETOPIC_NAME, configDoc["MQTT_ONETOPIC_NAME"].as<const char*>());
    }
    config->MQTT_PORT = configDoc["MQTT_PORT"].as<uint16_t>();
    config->FREQUENCY = configDoc["FREQUENCY"].as<uint32_t>();
    config->PIN_RX = configDoc["PIN_RX"].as<uint8_t>();
    config->PIN_TX = configDoc["PIN_TX"].as<uint8_t>();
    config->PIN_THERM = configDoc["PIN_THERM"].as<uint8_t>();
    config->SG_ENABLED = configDoc["SG_ENABLED"].as<const bool>();
    config->PIN_SG1 = configDoc["PIN_SG1"].as<uint8_t>();
    config->PIN_SG2 = configDoc["PIN_SG2"].as<uint8_t>();
    config->SG_RELAY_HIGH_TRIGGER = configDoc["SG_RELAY_HIGH_TRIGGER"].as<const bool>();
    config->PIN_ENABLE_CONFIG = configDoc["PIN_ENABLE_CONFIG"].as<uint8_t>();

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

    if(config->SSID_STATIC_IP)
    {
        configDoc["SSID_IP"] = config->SSID_IP;
        configDoc["SSID_SUBNET"] = config->SSID_SUBNET;
        configDoc["SSID_GATEWAY"] = config->SSID_GATEWAY;
        configDoc["SSID_PRIMARY_DNS"] = config->SSID_PRIMARY_DNS;
        configDoc["SSID_SECONDARY_DNS"] = config->SSID_SECONDARY_DNS;
    }

    configDoc["MQTT_SERVER"] = config->MQTT_SERVER;
    configDoc["MQTT_USERNAME"] = config->MQTT_USERNAME;
    configDoc["MQTT_PASSWORD"] = config->MQTT_PASSWORD;
    configDoc["MQTT_USE_JSONTABLE"] = config->MQTT_USE_JSONTABLE;
    configDoc["MQTT_USE_ONETOPIC"] = config->MQTT_USE_ONETOPIC;
    
    if(config->MQTT_USE_ONETOPIC)
    {
        configDoc["MQTT_ONETOPIC_NAME"] = config->MQTT_ONETOPIC_NAME;
    }

    configDoc["MQTT_PORT"] = config->MQTT_PORT;
    configDoc["FREQUENCY"] = config->FREQUENCY;
    configDoc["PIN_RX"] = config->PIN_RX;
    configDoc["PIN_TX"] = config->PIN_TX;
    configDoc["PIN_THERM"] = config->PIN_THERM;
    configDoc["SG_ENABLED"] = config->SG_ENABLED;
    configDoc["PIN_SG1"] = config->PIN_SG1;
    configDoc["PIN_SG2"] = config->PIN_SG2;
    configDoc["SG_RELAY_HIGH_TRIGGER"] = config->SG_RELAY_HIGH_TRIGGER;
    configDoc["PIN_ENABLE_CONFIG"] = config->PIN_ENABLE_CONFIG;
    
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