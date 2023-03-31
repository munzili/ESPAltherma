#ifndef CAN_BUS_H
#define CAN_BUS_H

#include "MQTT/mqttSerial.hpp"
#include "Config/config.hpp"
#include "CANDriver.hpp"
#include "CAN/Drivers/MCP2515.hpp"
#include "hpsu.hpp"

void canBus_setup();

void canBus_loop();

#endif