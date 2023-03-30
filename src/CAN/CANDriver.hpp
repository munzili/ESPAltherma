#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H

#include "Config/commandDef.hpp"

class CANDriver
{
public:
    virtual bool initInterface();
    virtual const char* sendCommandWithID(CommandDef cmd, bool setValue = false, int value = 0);
};

#endif