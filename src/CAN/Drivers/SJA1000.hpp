#ifndef DRIVER_SJA1000_H
#define DRIVER_SJA1000_H

//#include <CAN.h>
#include "CAN/CANDriver.hpp"
#include "CAN/CANPort.hpp"

class DriverSJA1000 : CANDriver
{
public:
    bool initInterface(CANPort* port, int baudrate);
    const char* sendCommandWithID(CommandDef* cmd, bool setValue = false, int value = 0);
};

#endif