#include "canBus.hpp"

CANDriver *driver = nullptr;

void canBus_setup()
{
    switch (config->CAN_IC)
    {
    case CanICTypes::MCP2515:
        driver = new DriverMCP2515();
        driver->initInterface();
        break;

    case CanICTypes::ELM327:
        driver = new DriverELM327();
        driver->initInterface();
        break;

    /*case CanICTypes::SJA1000:
        break;*/

    default:
        mqttSerial.println("No CAN Driver found");
        return;
    }

    driver->enableSniffing(config->CAN_SNIFFING_ENABLED);
}

void canBus_loop()
{
    if(driver != nullptr)
        driver->handleLoop();
}