#ifndef DRIVER_MCP2515_H
#define DRIVER_MCP2515_H

#include <SPI.h>
#include <107-Arduino-MCP2515.h>
#include "CAN/CANDriver.hpp"
#include "MQTT/mqttSerial.hpp"
#include "Config/config.hpp"

struct CanFrame
{
  uint32_t id;
  uint8_t  data[8];
  uint8_t  len;
};

void onReceiveBufferFull(uint32_t const, uint32_t const, uint8_t const *, uint8_t const);

class DriverMCP2515 : CANDriver
{
public:
    bool initInterface();
    const char* read();
    void write(const char *bytes, size_t size);
    void writePart(const char *bytes, size_t size);
    void setID(const uint16_t id);
    const char* sendCommandWithID(CommandDef* cmd, bool setValue = false, int value = 0);
};

#endif