#include "canBus.hpp"

bool canInited = false;

void canBus_setup(int8_t rxPin, int8_t txPin, uint16_t speed)
{
  /*CAN.setPins(rxPin, txPin);
  CAN.begin(speed * 1000);*/

  // Konfiguration des CAN-Controllers
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)txPin, (gpio_num_t)rxPin, TWAI_MODE_NORMAL);
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
  twai_timing_config_t t_config;

  switch (speed)
  {
  case 1:
    t_config = TWAI_TIMING_CONFIG_1KBITS();
    break;

  case 5:
    t_config = TWAI_TIMING_CONFIG_5KBITS();
    break;

  case 10:
    t_config = TWAI_TIMING_CONFIG_10KBITS();
    break;

  case 12:
    t_config = TWAI_TIMING_CONFIG_12_5KBITS();
    break;

  case 16:
    t_config = TWAI_TIMING_CONFIG_16KBITS();
    break;

  case 20:
    t_config = TWAI_TIMING_CONFIG_20KBITS();
    break;

  case 25:
    t_config = TWAI_TIMING_CONFIG_25KBITS();
    break;

  case 50:
    t_config = TWAI_TIMING_CONFIG_50KBITS();
    break;

  case 100:
    t_config = TWAI_TIMING_CONFIG_100KBITS();
    break;

  case 125:
    t_config = TWAI_TIMING_CONFIG_125KBITS();
    break;

  case 250:
    t_config = TWAI_TIMING_CONFIG_250KBITS();
    break;

  case 500:
    t_config = TWAI_TIMING_CONFIG_500KBITS();
    break;

  case 800:
    t_config = TWAI_TIMING_CONFIG_800KBITS();
    break;

  case 1000:
    t_config = TWAI_TIMING_CONFIG_1MBITS();
    break;

  default:
    mqttSerial.println("CAN-Bus init failed! E1");
    return; // error - wrong speed
    break;
  }

  // Initialisierung des CAN-Controllers
  if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
    mqttSerial.println("CAN-Bus init failed! E2");
    return;
  }

  if (twai_start() != ESP_OK) {
    mqttSerial.println("CAN-Bus init failed! E3");
    return;
  }

  canInited = true;
  mqttSerial.println("CAN-Bus inited");
}

ulong lastCANReading;

void canBus_loop()
{
  if(!canInited)
  {
    return;
  }

  ulong time = millis();
  if(time - lastCANReading > 5000)
  {
    lastCANReading = time;
    mqttSerial.println("Try CAN-Reading...");
  }

  twai_message_t message;

  if (twai_receive(&message, 0) != ESP_OK) {
    return; // no messages
  }

  if (!message.rtr)
  {
    mqttSerial.printf("Message from %i recieved:\n", message.identifier);

    for (uint8_t i = 0; i < message.data_length_code; i++)
    {
        mqttSerial.print(message.data[i], HEX);
        mqttSerial.print(" ");
    }

    mqttSerial.println();
  }

  return; // OK

  /*// try to parse packet
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
  }*/
}