#ifndef X10A_H
#define X10A_H

#include <HardwareSerial.h>
#include "RegistryBuffer.hpp"
#include "Config/config.hpp"
#include "MQTT/mqttSerial.hpp"
#include "MQTT/mqtt.hpp"
#include "arrayFunctions.hpp"
#include "comm.hpp"
#include "main.hpp"

extern HardwareSerial SerialX10A;
extern size_t registryBufferSize;
extern RegistryBuffer *registryBuffers;

void X10AEnd();

void initRegistries(RegistryBuffer** buffer, size_t& bufferSize, ParameterDef** parameters, size_t parametersLength);

void handleX10A(RegistryBuffer* buffer, size_t& bufferSize, ParameterDef** parameters, size_t parametersLength, bool sendValuesViaMQTT);

void X10AInit(int8_t rxPin, int8_t txPin);

#endif