#ifndef DRIVER_ELM327_H
#define DRIVER_ELM327_H

#include "CAN/CANDriver.hpp"
#include "Config/config.hpp"

class DriverELM327 : CANDriver
{
private:
    const char* read();
    void writePart(const char *bytes, size_t size);
    void write(const char *bytes, size_t size);
    void setID(const uint16_t id);

public:
    bool initInterface();
    void sendCommandWithID(CommandDef* cmd, bool setValue = false, int value = 0);
};

#endif