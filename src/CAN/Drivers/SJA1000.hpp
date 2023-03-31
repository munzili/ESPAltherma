#ifndef DRIVER_SJA1000_H
#define DRIVER_SJA1000_H

//#include <CAN.h>
#include "driver/twai.h"
#include "CAN/CANDriver.hpp"
#include "Config/config.hpp"
#include "MQTT/mqttSerial.hpp"

class DriverSJA1000 : CANDriver
{
public:
    bool initInterface();
    void sendCommandWithID(CommandDef* cmd, bool setValue = false, int value = 0);
};

#endif