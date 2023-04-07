#ifndef DRIVER_MCP2515_H
#define DRIVER_MCP2515_H

#include <SPI.h>
#include <107-Arduino-MCP2515.h>
#include "CAN/CANDriver.hpp"
#include "MQTT/mqttSerial.hpp"
#include "MQTT/mqtt.hpp"
#include "Config/config.hpp"

using namespace MCP2515;

#define CAN_MESSAGE_TIMEOUT 4 // define timout in seconds for message send to get answer

struct CanFrame
{
  uint32_t id;
  uint8_t  data[8];
  uint8_t  len;
};

struct CMDSendInfo
{
  bool pending = false;
  uint64_t timeMessageSend = 0;
  CommandDef* cmd;
};

class DriverMCP2515 : public CANDriver
{
private:
  ArduinoMCP2515* mcp2515;
  CMDSendInfo** cmdSendInfos;
  const SPISettings MCP2515x_SPI_SETTING{1000000, MSBFIRST, SPI_MODE0};
  uint16_t currentFrameId;
  bool sniffingEnabled = false;
  Mode currentMode;
  bool canInited = false;
  ulong lastTimeRunned = 0;

  bool setMode(Mode mode);
  static int HPSU_toSigned(uint16_t value, char* unit);
  bool getRate(const uint8_t mhz, const uint16_t speed, CanBitRate &rate);
  void sniffCAN(const uint32_t timestamp_us, const uint32_t id, const uint8_t *data, const uint8_t len);
  CommandDef* getCommandFromData(const uint8_t *data);

public:
  DriverMCP2515();
  bool initInterface();
  void setID(const uint16_t id);
  void handleLoop();
  void enableSniffing(bool value);
  void writeLoopbackTest();
  void sendCommand(CommandDef* cmd, bool setValue = false, int value = 0);
  void handleMQTTSetRequest(const String &label, const char *payload, const uint32_t length);
  void listenOnly(bool value = true);
  void onReceiveBufferFull(uint32_t const, uint32_t const, uint8_t const *, uint8_t const);
  void handleInterrupt();
};

#endif