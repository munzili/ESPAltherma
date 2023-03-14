#ifndef CONFIG_H
#define CONFIG_H
#include <stdint.h>
#include "parameterDef.h"
#include "commandDef.h"
#include "ArduinoJson.h"
#include <LittleFS.h>

#define CONFIG_FILE "/config.json"
#define MODELS_CONFIG_SIZE 1024*10

struct Config
{
    bool configStored;
    bool STANDALONE_WIFI;
    String SSID;
    String SSID_PASSWORD;
    bool SSID_STATIC_IP;
    String SSID_IP;
    String SSID_SUBNET;
    String SSID_GATEWAY;
    String SSID_PRIMARY_DNS;
    String SSID_SECONDARY_DNS;
    String MQTT_SERVER;
    String MQTT_USERNAME;
    String MQTT_PASSWORD;
    bool MQTT_USE_JSONTABLE;
    bool MQTT_USE_ONETOPIC;
    String MQTT_ONETOPIC_NAME;
    uint16_t MQTT_PORT;
    uint32_t FREQUENCY;
    uint8_t PIN_RX;
    uint8_t PIN_TX;
    uint8_t PIN_THERM;
    bool SG_ENABLED;
    uint8_t PIN_SG1;
    uint8_t PIN_SG2;
    bool SG_RELAY_HIGH_TRIGGER;
    bool CAN_ENABLED;
    uint8_t PIN_CAN_RX;
    uint8_t PIN_CAN_TX;
    uint16_t CAN_SPEED_KBPS;
    uint8_t PIN_ENABLE_CONFIG;
    size_t PARAMETERS_LENGTH;
    ParameterDef** PARAMETERS;
    size_t COMMANDS_LENGTH;
    CommandDef** COMMANDS;
    char* WEBUI_SELECTION_VALUES;

    ~Config()
    {
        if(PARAMETERS_LENGTH)
        {
            for (size_t i = 0; i < PARAMETERS_LENGTH; i++)
            {
                delete PARAMETERS[i];
            }
            delete[] PARAMETERS;
        }

        if(COMMANDS_LENGTH)
        {
            for (size_t i = 0; i < COMMANDS_LENGTH; i++)
            {
                delete COMMANDS[i];
            }
            delete[] COMMANDS;
        }
    }
};

Config* config = nullptr;

void readConfig()
{
    if(config != nullptr)
        delete config;

    config = new Config();

    if(!LittleFS.exists(CONFIG_FILE))
        return;

    File configFile = LittleFS.open(CONFIG_FILE, FILE_READ);
    DynamicJsonDocument configDoc(MODELS_CONFIG_SIZE);
    deserializeJson(configDoc, configFile);
    serializeJsonPretty(configDoc, Serial);
    configFile.close();

    config->configStored = true;
    config->STANDALONE_WIFI = configDoc["STANDALONE_WIFI"].as<const bool>();

    if(!config->STANDALONE_WIFI)
    {
        config->SSID = (char *)configDoc["SSID"].as<const char*>();
        config->SSID_PASSWORD = (char *)configDoc["SSID_PASSWORD"].as<const char*>();
        config->SSID_STATIC_IP = configDoc["SSID_STATIC_IP"].as<const bool>();
        if(config->SSID_STATIC_IP)
        {
            config->SSID_IP = (char *)configDoc["SSID_IP"].as<const char*>();
            config->SSID_SUBNET = (char *)configDoc["SSID_SUBNET"].as<const char*>();
            config->SSID_GATEWAY = (char *)configDoc["SSID_GATEWAY"].as<const char*>();
            config->SSID_PRIMARY_DNS = (char *)configDoc["SSID_PRIMARY_DNS"].as<const char*>();
            config->SSID_SECONDARY_DNS = (char *)configDoc["SSID_SECONDARY_DNS"].as<const char*>();
        }
    }

    config->MQTT_SERVER = (char *)configDoc["MQTT_SERVER"].as<const char*>();
    config->MQTT_USERNAME = (char *)configDoc["MQTT_USERNAME"].as<const char*>();
    config->MQTT_PASSWORD = (char *)configDoc["MQTT_PASSWORD"].as<const char*>();
    config->MQTT_USE_JSONTABLE = configDoc["MQTT_USE_JSONTABLE"].as<const bool>();
    config->MQTT_USE_ONETOPIC = configDoc["MQTT_USE_ONETOPIC"].as<const bool>();

    if(config->MQTT_USE_ONETOPIC)
    {
        config->MQTT_ONETOPIC_NAME = (char *)configDoc["MQTT_ONETOPIC_NAME"].as<const char*>();
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
    config->CAN_ENABLED = configDoc["CAN_ENABLED"].as<const bool>();
    config->PIN_CAN_RX = configDoc["PIN_CAN_RX"].as<uint8_t>();
    config->PIN_CAN_TX = configDoc["PIN_CAN_TX"].as<uint8_t>();
    config->CAN_SPEED_KBPS = configDoc["CAN_SPEED_KBPS"].as<uint8_t>();
    config->PIN_ENABLE_CONFIG = configDoc["PIN_ENABLE_CONFIG"].as<uint8_t>();

    JsonArray parameters = configDoc["PARAMETERS"].as<JsonArray>();
    config->PARAMETERS_LENGTH = parameters.size();
    config->PARAMETERS = new ParameterDef*[config->PARAMETERS_LENGTH];

    for(size_t i = 0; i < config->PARAMETERS_LENGTH; i++)
    {
        JsonArray parameter = parameters[i];
        config->PARAMETERS[i] = new ParameterDef(
            parameter[0].as<const int>(),
            parameter[1].as<const int>(),
            parameter[2].as<const int>(),
            parameter[3].as<const int>(),
            parameter[4].as<const int>(),
            parameter[5]);
    }

    JsonArray commands = configDoc["COMMANDS"].as<JsonArray>();
    config->COMMANDS_LENGTH = commands.size();
    config->COMMANDS = new CommandDef*[config->COMMANDS_LENGTH];

    for(size_t i = 0; i < config->COMMANDS_LENGTH; i++)
    {
        JsonArray command = commands[i];

        JsonArray commandBytes = command[1];
        byte commandArray[] = {
            commandBytes[0],
            commandBytes[1],
            commandBytes[2],
            commandBytes[3],
            commandBytes[4],
            commandBytes[5],
            commandBytes[6]
        };

        config->COMMANDS[i] = new CommandDef(
            command[0],
            commandArray/*,
            command[3].as<const uint16_t>(),
            command[4].as<const float>(),
            command[5].as<const bool>(),
            command[6].as<char*>(),
            command[7].as<char*>(),
            command[8]*/);
    }

    config->WEBUI_SELECTION_VALUES = (char *)configDoc["WEBUI_SELECTION_VALUES"].as<const char*>();
}


void saveConfig()
{
    DynamicJsonDocument configDoc(MODELS_CONFIG_SIZE);
    configDoc["STANDALONE_WIFI"] = config->STANDALONE_WIFI;

    if(!config->STANDALONE_WIFI)
    {
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
    configDoc["CAN_ENABLED"] = config->CAN_ENABLED;
    configDoc["PIN_CAN_RX"] = config->PIN_CAN_RX;
    configDoc["PIN_CAN_TX"] = config->PIN_CAN_TX;
    configDoc["CAN_SPEED_KBPS"] = config->CAN_SPEED_KBPS;
    configDoc["SG_RELAY_HIGH_TRIGGER"] = config->SG_RELAY_HIGH_TRIGGER;
    configDoc["PIN_ENABLE_CONFIG"] = config->PIN_ENABLE_CONFIG;

    JsonArray parameters = configDoc.createNestedArray("PARAMETERS");
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
Serial.println("SAVING COMMANDS!!");
Serial.print("Commands length: ");
Serial.println(config->COMMANDS_LENGTH);
    JsonArray commands = configDoc.createNestedArray("COMMANDS");
    for(size_t i = 0; i < config->COMMANDS_LENGTH; i++)
    {
        JsonArray command = commands.createNestedArray();
Serial.print("Label: ");
Serial.println(config->COMMANDS[i]->label);
        command.add(config->COMMANDS[i]->label);

        JsonArray commandBytes = command.createNestedArray();

        for (uint8_t i = 0; i < COMMAND_BYTE_LENGTH; i++)
        {
Serial.print("Bytes Calc: ");
Serial.println(config->COMMANDS[i]->command[i]);
            commandBytes.add(config->COMMANDS[i]->command[i]);
        }

        /*command.add(config->COMMANDS[i]->id);
        command.add(config->COMMANDS[i]->divisor);
        command.add(config->COMMANDS[i]->writable);
        command.add(config->COMMANDS[i]->unit);
        command.add(config->COMMANDS[i]->type);
        command.add(config->COMMANDS[i]->valueCode);*/
    }

    configDoc["WEBUI_SELECTION_VALUES"] = config->WEBUI_SELECTION_VALUES;

    File configFile = LittleFS.open(CONFIG_FILE, FILE_WRITE);
    serializeJsonPretty(configDoc, Serial);
    serializeJson(configDoc, configFile);
    configFile.close();
}

#endif