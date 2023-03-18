#ifndef PORT_SJA1000_H
#define PORT_SJA1000_H

#include "ArduinoC.hpp"
#include "CAN/CANPort.hpp"

class PortSJA1000 : CANPort
{
public:
    PortSJA1000(int8_t rxPin, int8_t txPin);

    void write(const char* bytes);
    const char* read();
};

#endif