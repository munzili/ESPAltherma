#ifndef config_h
#define config_h
#include <stdint.h>

#define MAX_MSG_SIZE 4096//max size of the json message sent in mqtt 

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
    uint8_t PARAMETERS[256];
} Config;

Config config;
#endif