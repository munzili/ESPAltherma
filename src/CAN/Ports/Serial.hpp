#ifndef PORT_SERIAL_H
#define PORT_SERIAL_H

#include "ArduinoC.hpp"
#include "CAN/CANPort.hpp"

class PortSerial : CANPort
{
public:
    PortSerial(int8_t uartNr, uint8_t baudrate);

    void setID(const uint16_t id);
    void writePart(const char* bytes, size_t size);
    void writePart(const byte* bytes, size_t size);
    void write(const char* bytes, size_t size);
    void write(const byte* bytes, size_t size);
    const char* read();
};

#endif