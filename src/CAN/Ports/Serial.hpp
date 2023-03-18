#ifndef PORT_SERIAL_H
#define PORT_SERIAL_H

#include <BluetoothSerial.h>
#include "ArduinoC.hpp"
#include "CAN/CANPort.hpp"

class PortSerial : CANPort
{
public:
    PortSerial(char* btName);
    PortSerial(int8_t rxPin, int8_t txPin, uint8_t baudrate);

    void write(const char* bytes);
    const char* read();
};

#endif