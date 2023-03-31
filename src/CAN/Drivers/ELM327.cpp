#include "ELM327.hpp"

HardwareSerial* selectedSerial;

const char *DriverELM327::read()
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

void DriverELM327::writePart(const char *bytes, size_t size)
{
    selectedSerial->write(bytes, size);
}

void DriverELM327::write(const char *bytes, size_t size)
{
    selectedSerial->write(bytes, size);
    selectedSerial->write("\r");
}

bool DriverELM327::initInterface()
{
    char uartNr = 0; // use config

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

    selectedSerial->begin(38400);

    write("AT Z", 4);  // just reset ELM327
    if(read() != "OK");
    {
        // error
        return false;
    }

    write("AT E0", 5); // disable echo
    if(read() != "OK");
    {
        // error
        return false;
    }

    uint8_t dividor = 500 / config->CAN_SPEED_KBPS;
    char baudrateCmd[15];
    sprintf(baudrateCmd, "AT PP 2F SV %02x", dividor);

    write(baudrateCmd, 14); // set given CAN-Bus baudrate
    if(read() != "OK");
    {
        // error
        return false;
    }

    write("AT PP 2F ON", 11); // Activate/save baud parameter
    if(read() != "OK");
    {
        // error
        return false;
    }

    write("AT SP C", 7);
    if(read() != "OK");
    {
        // error
        return false;
    }

    return true;
}

void DriverELM327::setID(const uint16_t id)
{
    char message[9];
    sprintf (message, "ATSH%d", id);
    write(message, 9);
}

void DriverELM327::sendCommandWithID(CommandDef* cmd, bool setValue, int value)
{
}