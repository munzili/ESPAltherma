#include "Serial.hpp"

HardwareSerial* selectedSerial;

PortSerial::PortSerial(int8_t uartNr, uint8_t baudrate)
{
    switch (uartNr)
    {
    case 1:
        selectedSerial = &Serial1;
        break;

    case 2:
        selectedSerial = &Serial2;
        break;

    default:
        selectedSerial = &Serial;
        break;
    }

    selectedSerial->begin(baudrate);
}

void PortSerial::setID(const uint16_t id)
{
    char message[9];
    sprintf (message, "ATSH%d", id);
    write(message, 9);
}

void PortSerial::writePart(const char *bytes, size_t size)
{
    selectedSerial->write(bytes, size);
}

void PortSerial::write(const char *bytes, size_t size)
{
    selectedSerial->write(bytes, size);
    selectedSerial->write("\r");
}

const char* PortSerial::read()
{
    int packetSize = selectedSerial->available();

    if (packetSize)
    {
        char result[packetSize] = "";

        selectedSerial->read(result, packetSize);

        return result;
    }

    return nullptr;
}
