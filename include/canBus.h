#ifndef CAN_BUS_H
#define CAN_BUS_H

#include <CAN.h>
#include "mqttserial.h"
#include "hpsu.h"

void canBus_setup(int8_t rxPin, int8_t txPin, uint8_t speed)
{
  CAN.setPins(rxPin, txPin);
  CAN.begin(speed * 1000);

  mqttSerial.println("CAN-Bus inited");
}

void canBus_loop()
{
  // try to parse packet
  int packetSize = CAN.parsePacket();

  if (packetSize) {
    // received a packet
    mqttSerial.print("Received ");

    if (CAN.packetExtended()) {
      mqttSerial.print("extended ");
    }

    if (CAN.packetRtr()) {
      // Remote transmission request, packet contains no data
      mqttSerial.print("RTR ");
    }

    mqttSerial.print("packet with id 0x");
    mqttSerial.print(CAN.packetId(), HEX);

    if (CAN.packetRtr()) {
      mqttSerial.print(" and requested length ");
      mqttSerial.println(CAN.packetDlc());
    } else {
      mqttSerial.print(" and length ");
      mqttSerial.println(packetSize);
    }

    // only print packet data for non-RTR packets
    while (CAN.available()) {
      mqttSerial.print(CAN.read(), HEX);
      mqttSerial.print(" ");
    }
    mqttSerial.println();

    mqttSerial.println();
  }
}

#endif