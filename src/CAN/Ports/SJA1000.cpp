#include "SJA1000.hpp"

bool packageStarted = false;
uint16_t headerId = 0;

PortSJA1000::PortSJA1000(int8_t rxPin, int8_t txPin)
{
    CAN.setPins(rxPin, txPin);
}

void PortSJA1000::setID(const uint16_t id)
{
    headerId = id;
}

void PortSJA1000::writePart(const byte *bytes, size_t size)
{
    if(!packageStarted)
    {
        CAN.beginPacket(headerId, COMMAND_BYTE_LENGTH);
        packageStarted = true;
    }

    CAN.write(bytes, size);
}

void PortSJA1000::write(const byte *bytes, size_t size)
{
    if(!packageStarted)
        writePart(bytes, size);
    else
        CAN.write(bytes, size);

    if(!CAN.endPacket())
    {
        // error
    }
}

const char* PortSJA1000::read()
{
    int packetSize = CAN.parsePacket();

    if (packetSize)
    {
        char result[packetSize] = "";
        uint16_t counter = 0;

        while (CAN.available()) {
            result[counter] = (char)CAN.read();
            counter++;
        }

        return result;
    }

    return nullptr;
}