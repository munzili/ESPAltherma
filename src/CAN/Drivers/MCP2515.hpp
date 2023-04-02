#ifndef DRIVER_MCP2515_H
#define DRIVER_MCP2515_H

#include <SPI.h>
#include <107-Arduino-MCP2515.h>
#include "CAN/CANDriver.hpp"
#include "MQTT/mqttSerial.hpp"
#include "Config/config.hpp"

using namespace MCP2515;

struct CanFrame
{
  uint32_t id;
  uint8_t  data[8];
  uint8_t  len;
};

class DriverMCP2515 : public CANDriver
{
private:
  ArduinoMCP2515* mcp2515;
  const SPISettings MCP2515x_SPI_SETTING{1000000, MSBFIRST, SPI_MODE0};
  uint16_t currentFrameId;
  bool sniffMode = false;
  Mode currentMode;
  bool canInited = false;

  bool setMode(Mode mode);
  static int HPSU_toSigned(uint16_t value, char* unit);
  bool getRate(const uint8_t mhz, const uint16_t speed, CanBitRate &rate);
  void sniffCAN(const uint32_t timestamp_us, const uint32_t id, const uint8_t *data, const uint8_t len);

public:
  DriverMCP2515();
  bool initInterface();
  void setID(const uint16_t id);
  void writeLoopbackTest();
  void sendCommandWithID(CommandDef* cmd, bool setValue = false, int value = 0);
  void listenOnly(bool value = true);
  void onReceiveBufferFull(uint32_t const, uint32_t const, uint8_t const *, uint8_t const);
  void handleInterrupt();
};

#endif