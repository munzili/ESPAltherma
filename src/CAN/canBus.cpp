#include "canBus.hpp"

CANDriver *driver;

void canBus_setup()
{
    switch (config->CAN_IC)
    {
    case CanICTypes::MCP2515:
        driver = new DriverMCP2515();
        driver->initInterface();
        break;

    case CanICTypes::ELM327:
        break;

    case CanICTypes::SJA1000:
        break;

    default:
        mqttSerial.println("No CAN Driver found");
        break;
    }
}

void canBus_loop()
{
    switch (config->CAN_IC)
    {
    case CanICTypes::MCP2515:

        break;

    case CanICTypes::ELM327:
        break;

    case CanICTypes::SJA1000:
        break;
    }
}