#ifndef X10A_H
#define X10A_H

#include <HardwareSerial.h>
#ifdef ARDUINO_ARCH_ESP8266
#include <SoftwareSerial.h>
#endif

#include "registryBuffer.hpp"
#include "Config/config.hpp"
#include "debugSerial.hpp"
#include "MQTT/mqtt.hpp"
#include "arrayFunctions.hpp"
#include "config.hpp"
#include "comm.hpp"
#include "main.hpp"

#ifdef ARDUINO_ARCH_ESP8266
extern SoftwareSerial SerialX10A;
#define SERIAL_CONFIG (SWSERIAL_8E1)
#define DEFINE_SerialX10A SoftwareSerial SerialX10A
#else
extern HardwareSerial SerialX10A;
#define SERIAL_CONFIG (SERIAL_8E1)
#define DEFINE_SerialX10A HardwareSerial SerialX10A(1)
#endif

extern size_t registryBufferSize;
extern RegistryBuffer *registryBuffers;

enum HandleState : uint8_t {
    Stopped,
    Running
};

void x10a_end();

void x10a_initRegistries(RegistryBuffer** buffer, size_t& bufferSize);

void x10a_handle(RegistryBuffer* buffer, const size_t& bufferSize, const bool sendValuesViaMQTT);

void x10a_init(X10A_Config* X10AConfig, const bool disableMQTTLogMessagesToInit);

void x10a_loop();

#endif