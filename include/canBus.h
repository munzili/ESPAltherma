#ifndef CAN_BUS_H
#define CAN_BUS_H

#include <ESP32CAN.h>
#include <CAN_config.h>
#include "mqttserial.h"

CAN_device_t CAN_cfg;
const int rx_queue_size = 10;

void canBus_setup(int8_t rxPin, int8_t txPin, uint16_t speed)
{
  CAN_cfg.speed = (CAN_speed_t)speed;
  CAN_cfg.tx_pin_id = (gpio_num_t)txPin;
  CAN_cfg.rx_pin_id = (gpio_num_t)rxPin;
  CAN_cfg.rx_queue = xQueueCreate(rx_queue_size, sizeof(CAN_frame_t));
  ESP32Can.CANInit();
  mqttSerial.println("CAN-Bus inited");
}

void canBus_loop()
{
  CAN_frame_t rx_frame;

  unsigned long currentMillis = millis();

  // Receive next CAN frame from queue
  if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE) {

    if (rx_frame.FIR.B.FF == CAN_frame_std) {
      mqttSerial.printf("New standard frame");
    }
    else {
      mqttSerial.printf("New extended frame");
    }

    if (rx_frame.FIR.B.RTR == CAN_RTR) {
      mqttSerial.printf(" RTR from 0x%08X, DLC %d\r\n", rx_frame.MsgID,  rx_frame.FIR.B.DLC);
    }
    else {
      mqttSerial.printf(" from 0x%08X, DLC %d, Data ", rx_frame.MsgID,  rx_frame.FIR.B.DLC);
      for (int i = 0; i < rx_frame.FIR.B.DLC; i++) {
        mqttSerial.printf("0x%02X ", rx_frame.data.u8[i]);
      }
      mqttSerial.printf("\n");
    }
  }
}

#endif