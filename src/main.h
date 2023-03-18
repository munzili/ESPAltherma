#ifndef MAIN_H
#define MAIN_H

#include <PubSubClient.h>

#include "ArduinoC.h"
#include "Config/config.h"
#include "MQTT/mqttConfig.h"
#include "MQTT/mqttSerial.h"
#include "X10A/converters.h"
#include "X10A/comm.h"
#include "MQTT/mqtt.h"
#include "WebUI/webui.h"
#include "wireless.h"
#include "persistence.h"
#include "X10A/X10A.h"
#include "CAN/canBus.h"
#include "arrayFunctions.h"

MQTTSerial mqttSerial;

#if defined(ARDUINO_M5Stick_C) || defined(ARDUINO_M5Stick_C_Plus)
long LCDTimeout = 40000;//Keep screen ON for 40s then turn off. ButtonA will turn it On again.
#endif

uint8_t getFragmentation();

//Converts to string and add the value to the JSON message
void updateValues(ParameterDef *labelDef);

void extraLoop();

void setupScreen();

void IRAM_ATTR restartInStandaloneWifi();

void setup();

void waitLoop(uint ms);

void loop();
#endif