#ifndef PORT_SJA1000_H
#define PORT_SJA1000_H

#include <CAN.h>
#include "ArduinoC.hpp"
#include "CAN/CANPort.hpp"
#include "Config/commandDef.hpp"

class PortSJA1000 : CANPort
{
public:
    PortSJA1000(int8_t rxPin, int8_t txPin);

    void setID(const uint16_t id);
    void writePart(const char* bytes, size_t size);
    void writePart(const byte* bytes, size_t size);
    void write(const char* bytes, size_t size);
    void write(const byte* bytes, size_t size);
    const char* read();
};

#endif