#ifndef CAN_BUS_H
#define CAN_BUS_H

#include <CAN.h>
#include "MQTT/mqttSerial.hpp"
#include "hpsu.hpp"

void canBus_setup(int8_t rxPin, int8_t txPin, uint8_t speed);

void canBus_loop();

#endif