#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H

#include "Config/commandDef.hpp"
#include "MQTT/mqtt.hpp"

class CANDriver
{
public:
    virtual bool initInterface();
    virtual void handleLoop();
    virtual void sendCommandWithID(CommandDef *cmd, bool setValue = false, int value = 0);
};

#endif