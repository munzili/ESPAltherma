#ifndef COMM_H
#define COMM_H

#include <HardwareSerial.h>
#include "ArduinoC.hpp"
#include "X10A.hpp"
#include "MQTT/mqttSerial.hpp"

#define SER_TIMEOUT 300 //leave 300ms for the machine to answer

char getCRC(char *src, int len);

bool queryRegistry(RegistryBuffer *registryBuffer);

#endif