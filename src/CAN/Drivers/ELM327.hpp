#ifndef DRIVER_ELM327_H
#define DRIVER_ELM327_H

#include "CAN/CANDriver.hpp"
#include "CAN/CANPort.hpp"

class DriverELM327 : CANDriver
{
public:
    bool initInterface(CANPort* port, int baudrate);
    const char* sendCommandWithID(CommandDef* cmd, bool setValue);
};

#endif