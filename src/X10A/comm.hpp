#ifndef COMM_H
#define COMM_H

#include <HardwareSerial.h>
#include "ArduinoC.hpp"
#include "X10A.hpp"
#include "MQTT/mqttSerial.hpp"

#define SER_TIMEOUT 300 //leave 300ms for the machine to answer
#define MAX_BUFFER_SIZE 32 //max bytes returned from X10A Port

struct RegistryBuffer {
  char RegistryID;
  char Buffer[MAX_BUFFER_SIZE];
  bool Success;
  char CRC;
};

char getCRC(char *src, int len);

bool queryRegistry(RegistryBuffer *registryBuffer);

#endif