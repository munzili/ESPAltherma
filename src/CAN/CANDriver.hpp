#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H

#include "CANPort.hpp"
#include "Config/commandDef.hpp"

class CANDriver
{
protected:
    CANPort* canPort;

public:
    virtual bool initInterface(CANPort* port, int baudrate);
    virtual const char* sendCommandWithID(CommandDef cmd, bool setValue);
};

#endif