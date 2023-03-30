#ifndef DRIVER_MCP2515_H
#define DRIVER_MCP2515_H

#include <SPI.h>
#include <107-Arduino-MCP2515.h>
#include "CAN/CANDriver.hpp"
#include "MQTT/mqttSerial.hpp"
#include "Config/config.hpp"

class DriverMCP2515 : CANDriver
{
public:
    bool initInterface();
    const char* sendCommandWithID(CommandDef* cmd, bool setValue = false, int value = 0);
};

#endif