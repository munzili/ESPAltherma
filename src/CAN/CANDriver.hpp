#ifndef CAN_DRIVER_H
#define CAN_DRIVER_H

#include "Config/commandDef.hpp"
#include "MQTT/mqtt.hpp"

class CANDriver
{
public:
    virtual bool initInterface();
    virtual const char* read();
    virtual void write(const char *bytes, size_t size);
    virtual void writePart(const char *bytes, size_t size);
    virtual void setID(const uint16_t id);
    const char* sendCommandWithID(CommandDef *cmd, bool setValue = false, int value = 0);
};

#endif