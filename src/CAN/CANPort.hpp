#ifndef CAN_PORT_H
#define CAN_PORT_H

#include "ArduinoC.hpp"

class CANPort
{
public:
    virtual void setID(const uint16_t id);
    virtual void writePart(const char* bytes, size_t size);
    virtual void writePart(const byte* bytes, size_t size);
    virtual void write(const char* bytes, size_t size);
    virtual void write(const byte* bytes, size_t size);
    virtual const char* read();
};

#endif