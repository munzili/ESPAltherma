#include "MCP2515.hpp"

// tranlate from fhemHPSU

using namespace MCP2515;

DriverMCP2515* self;

void DriverMCP2515::sniffCAN(const uint32_t timestamp_us, const uint32_t id, const uint8_t *data, const uint8_t len)
{
  char resultText[128] = "";
  sprintf(resultText, "CAN [ %i ] ID", timestamp_us);

  if(id & MCP2515::CAN_EFF_BITMASK) strcat(resultText, "(EXT)");
  if(id & MCP2515::CAN_RTR_BITMASK) strcat(resultText, "(RTR)");

  sprintf(resultText + strlen(resultText), " %02X DATA[%i] ", id, len);

  std::for_each(data,
                  data+len,
                  [resultText](uint8_t const elem) mutable {
                      sprintf(resultText + strlen(resultText), "%02X ", elem);
                  });

  mqttSerial.println(resultText);
}

bool DriverMCP2515::setMode(Mode mode)
{
  bool success = false;

  switch (mode)
  {
  case Mode::Normal:
    success = mcp2515->setNormalMode();
    break;

  case Mode::Sleep:
    success = mcp2515->setSleepMode();
    break;

  case Mode::Loopback:
    success = mcp2515->setLoopbackMode();
    break;

  case Mode::ListenOnly:
    success = mcp2515->setListenOnlyMode();
    break;

  case Mode::Config:
    success = mcp2515->setConfigMode();
    break;
  }

  if(success)
    currentMode = mode;

  return success;
}

void DriverMCP2515::writeLoopbackTest()
{
  mqttSerial.println("CAN running loopback test");

  Mode modeBeforeTest = currentMode;
  setMode(Mode::Loopback);

  CanFrame const test_frame_1 = { 0x00000001, {0}, 0 };                                              /* Minimum (no) payload */
  CanFrame const test_frame_2 = { 0x00000002, {0xCA, 0xFE, 0xCA, 0xFE, 0, 0, 0, 0}, 4 };             /* Between minimum and maximum payload */
  CanFrame const test_frame_3 = { 0x00000003, {0xCA, 0xFE, 0xCA, 0xFE, 0xCA, 0xFE, 0xCA, 0xFE}, 8 }; /* Maximum payload */
  CanFrame const test_frame_4 = { 0x40000004, {0}, 0 };                                              /* RTR frame */
  CanFrame const test_frame_5 = { 0x000007FF, {0}, 0 };                                              /* Highest standard 11 bit CAN address */
  CanFrame const test_frame_6 = { 0x80000000, {0}, 0 };                                              /* Lowest extended 29 bit CAN address */
  CanFrame const test_frame_7 = { 0x9FFFFFFF, {0}, 0 };                                              /* Highest extended 29 bit CAN address */

  std::array<CanFrame, 7> const CAN_TEST_FRAME_ARRAY =
  {
    test_frame_1,
    test_frame_2,
    test_frame_3,
    test_frame_4,
    test_frame_5,
    test_frame_6,
    test_frame_7
  };

  std::for_each(CAN_TEST_FRAME_ARRAY.cbegin(),
                CAN_TEST_FRAME_ARRAY.cend(),
                [this](CanFrame const frame)
                {
                if(!mcp2515->transmit(frame.id, frame.data, frame.len)) {
                    mqttSerial.println("ERROR TX");
                }
                delay(10);
                });

  setMode(modeBeforeTest);
}

int DriverMCP2515::HPSU_toSigned(uint16_t value, char* unit)
{
  if(unit == "deg" || unit == "value_code_signed")
  {
    int newValue = value & 0xFFFF;
    return (newValue ^ 0x8000) - 0x8000;
  }
  else
  {
    return value;
  }
}

CommandDef* DriverMCP2515::getCommandFromData(const uint8_t *data)
{
  bool extended = data[2] == 0xFA;
  CommandDef* recievedCommand = nullptr;

  for(size_t i = 0; i < config->COMMANDS_LENGTH; i++)
  {
    //Byte 3 == FA
    //31 00 FA 0B D1 00 00   <- $CANMsg
    //      |------| -> len: 3 byte
    //      ^pos: 2
    if(extended &&
       config->COMMANDS[i]->command[2] == data[2] &&
       config->COMMANDS[i]->command[3] == data[3] &&
       config->COMMANDS[i]->command[4] == data[4])
    {
      recievedCommand = config->COMMANDS[i];
      break;
    }
    //Byte 3 != FA
    //31 00 05 00 00 00 00   <- $CANMsg
    //      || -> len: 1 byte
    //      ^pos: 2
    else if(!extended && config->COMMANDS[i]->command[2] == data[2])
    {
      recievedCommand = config->COMMANDS[i];
      break;
    }
  }

  return recievedCommand;
}

void DriverMCP2515::onReceiveBufferFull(const uint32_t timestamp_us, const uint32_t id, const uint8_t *data, const uint8_t len)
{
  if(!canInited)
      return;

  if(sniffMode || currentMode == Mode::Loopback)
  {
    sniffCAN(timestamp_us, id, data, len);

    if(currentMode == Mode::Loopback)
      return;
  }

  if(len < 2)
    return;

  bool extended = data[2] == 0xFA;

  bool valid = false;
  CommandDef* recievedCommand = getCommandFromData(data);

  // if we got a message that we shouldnt handle, skip it
  if(recievedCommand == nullptr)
    return;

  for(size_t i = 0; i < config->COMMANDS_LENGTH; i++)
  {
    if(cmdSendInfos[i]->cmd == recievedCommand)
    {
      // if we didnt fetch the infos, ignore it
      if(!cmdSendInfos[i]->pending)
      {
        return;
      }

      cmdSendInfos[i]->pending = false;
      break;
    }
  }

  byte valByte1 = 0;
  byte valByte2 = 0;

  if (extended)  // -> Byte3 eq FA
  {
    //20 0A FA 01 D6 00 D9   <- $CANMsg
    //               |  ^pos: 6
    //               ^pos: 5
    //   t_hs - 21,7

    valByte1 = data[5];
    valByte2 = data[6];
  }
  else
  {
    //20 0A 0E 01 E8 00 00   <- $CANMsg
    //         |  ^pos: 4
    //         ^pos: 3
    //   t_dhw - 48,8Â°
    valByte1 = data[3];
    valByte2 = data[4];
  }

  int value;

  if(recievedCommand->type == "int")
  {
    value = HPSU_toSigned(valByte1, recievedCommand->unit);
  }
  else if(recievedCommand->type == "value")
  {
    value = valByte1;
    //example: mode_01 val 4       -> 31 00 FA 01 12 04 00
  }
  else if(recievedCommand->type == "longint")
  {
    value = HPSU_toSigned(valByte2 + valByte1 * 0x0100, recievedCommand->unit);
    //example: one_hot_water val 1 -> 31 00 FA 01 44 00 01
    //                                                   ^
  }
  else if(recievedCommand->type == "float")
  {
    value = HPSU_toSigned(valByte2 + valByte1 * 0x0100, recievedCommand->unit);
  }
  else
  {
    return;
  }

  value /= recievedCommand->divisor;

  String valueCodeKey = String(value);

  if(recievedCommand->valueCodeSize > 0)
  {
    for (byte counter = 0; counter < recievedCommand->valueCodeSize; counter++)
    {
      if(recievedCommand->valueCode[counter]->value.toInt() == value)
      {
        valueCodeKey = recievedCommand->valueCode[counter]->key;
        break;
      }
    }
  }

  if(strlen(recievedCommand->unit) > 0)
  {
      if(strcmp(recievedCommand->unit, "deg") == 0)
      {
        valueCodeKey += " °C";
      }
      else if(strcmp(recievedCommand->unit, "percent") == 0)
      {
        valueCodeKey += " %";
      }
      else if(strcmp(recievedCommand->unit, "bar") == 0)
      {
        valueCodeKey += " bar";
      }
      else if(strcmp(recievedCommand->unit, "kwh") == 0)
      {
        valueCodeKey += " kWh";
      }
      else if(strcmp(recievedCommand->unit, "kw") == 0)
      {
        valueCodeKey += " kW";
      }
      else if(strcmp(recievedCommand->unit, "w") == 0)
      {
        valueCodeKey += " W";
      }
      else if(strcmp(recievedCommand->unit, "sec") == 0)
      {
        valueCodeKey += " sec";
      }
      else if(strcmp(recievedCommand->unit, "min") == 0)
      {
        valueCodeKey += " min";
      }
      else if(strcmp(recievedCommand->unit, "hour") == 0)
      {
        valueCodeKey += " h";
      }
      else if(strcmp(recievedCommand->unit, "lh") == 0)
      {
        valueCodeKey += " lh";
      }
  }

  if(config->MQTT_USE_ONETOPIC)
  {
    client.publish((config->MQTT_TOPIC_NAME + config->MQTT_ONETOPIC_NAME + config->CAN_MQTT_TOPIC_NAME + recievedCommand->label).c_str(), valueCodeKey.c_str());
  }
  else
  {
    client.publish((config->MQTT_TOPIC_NAME + config->CAN_MQTT_TOPIC_NAME + recievedCommand->label).c_str(), valueCodeKey.c_str());
  }

  mqttSerial.printf("CAN Data recieved %s: %s\n",  recievedCommand->label, valueCodeKey.c_str());
}


void DriverMCP2515::handleInterrupt()
{
  if(!canInited)
    return;

  mcp2515->onExternalEventHandler();
}

bool DriverMCP2515::getRate(const uint8_t mhz, const uint16_t speed, CanBitRate &rate)
{
  bool found = true;

  if(mhz == 8)
  {
    switch (config->CAN_SPEED_KBPS)
    {
    case 10:
      rate = CanBitRate::BR_10kBPS_8MHZ;
      break;

    case 20:
      rate = CanBitRate::BR_20kBPS_8MHZ;
      break;

    case 50:
      rate = CanBitRate::BR_50kBPS_8MHZ;
      break;

    case 100:
      rate = CanBitRate::BR_100kBPS_8MHZ;
      break;

    case 125:
      rate = CanBitRate::BR_125kBPS_8MHZ;
      break;

    case 250:
      rate = CanBitRate::BR_250kBPS_8MHZ;
      break;

    case 500:
      rate = CanBitRate::BR_500kBPS_8MHZ;
      break;

    case 800:
      rate = CanBitRate::BR_800kBPS_8MHZ;
      break;

    case 1000:
      rate = CanBitRate::BR_1000kBPS_8MHZ;
      break;

    default:
      found = false;
      break;
    }
  }
  else if(mhz == 10)
  {
    switch (config->CAN_SPEED_KBPS)
    {
    case 10:
      rate = CanBitRate::BR_10kBPS_10MHZ;
      break;

    case 20:
      rate = CanBitRate::BR_20kBPS_10MHZ;
      break;

    case 50:
      rate = CanBitRate::BR_50kBPS_10MHZ;
      break;

    case 100:
      rate = CanBitRate::BR_100kBPS_10MHZ;
      break;

    case 125:
      rate = CanBitRate::BR_125kBPS_10MHZ;
      break;

    case 250:
      rate = CanBitRate::BR_250kBPS_10MHZ;
      break;

    case 500:
      rate = CanBitRate::BR_500kBPS_10MHZ;
      break;

    case 1000:
      rate = CanBitRate::BR_1000kBPS_10MHZ;
      break;

    default:
      found = false;
      break;
    }
  }
  else if(mhz == 12)
  {
    switch (config->CAN_SPEED_KBPS)
    {
    case 10:
      rate = CanBitRate::BR_10kBPS_12MHZ;
      break;

    case 20:
      rate = CanBitRate::BR_20kBPS_12MHZ;
      break;

    case 50:
      rate = CanBitRate::BR_50kBPS_12MHZ;
      break;

    case 100:
      rate = CanBitRate::BR_100kBPS_12MHZ;
      break;

    case 125:
      rate = CanBitRate::BR_125kBPS_12MHZ;
      break;

    case 250:
      rate = CanBitRate::BR_250kBPS_12MHZ;
      break;

    case 500:
      rate = CanBitRate::BR_500kBPS_12MHZ;
      break;

    case 1000:
      rate = CanBitRate::BR_1000kBPS_12MHZ;
      break;

    default:
      found = false;
      break;
    }
  }
  else if(mhz == 16)
  {
    switch (config->CAN_SPEED_KBPS)
    {
    case 10:
      rate = CanBitRate::BR_10kBPS_16MHZ;
      break;

    case 20:
      rate = CanBitRate::BR_20kBPS_16MHZ;
      break;

    case 50:
      rate = CanBitRate::BR_50kBPS_16MHZ;
      break;

    case 100:
      rate = CanBitRate::BR_100kBPS_16MHZ;
      break;

    case 125:
      rate = CanBitRate::BR_125kBPS_16MHZ;
      break;

    case 250:
      rate = CanBitRate::BR_250kBPS_16MHZ;
      break;

    case 500:
      rate = CanBitRate::BR_500kBPS_16MHZ;
      break;

    case 800:
      rate = CanBitRate::BR_800kBPS_16MHZ;
      break;

    case 1000:
      rate = CanBitRate::BR_1000kBPS_16MHZ;
      break;

    default:
      found = false;
      break;
    }
  }
  else
  {
    found = false;
  }

  return found;
}

void DriverMCP2515::handleMQTTSetRequest(String label, byte *payload, unsigned int length)
{
  mqttSerial.printf("CAN: Got MQTT SET request for %s, %s\n", label, String(payload, length));
}

DriverMCP2515::DriverMCP2515()
{
  self = this;

  mcp2515 = new ArduinoMCP2515([this]()
                              {
                                SPI.beginTransaction(MCP2515x_SPI_SETTING);
                                digitalWrite(config->CAN_SPI.PIN_CS, LOW);
                              },
                              [this]()
                              {
                                digitalWrite(config->CAN_SPI.PIN_CS, HIGH);
                                SPI.endTransaction();
                              },
                              [this](uint8_t const dataByte) { return SPI.transfer(dataByte); },
                              micros,
                              [this](const uint32_t timestamp_us, const uint32_t id, const uint8_t *data, const uint8_t len)
                              {
                                onReceiveBufferFull(timestamp_us, id, data, len);
                              },
                              nullptr);
}

bool DriverMCP2515::initInterface()
{
  CanBitRate rate;

  bool ratePossible = getRate(config->CAN_SPI.IC_MHZ, config->CAN_SPEED_KBPS, rate);

  if(!ratePossible) // test if we can write something to the MCP2515 (is a device connected?)
  {
      mqttSerial.println("CAN-Bus init failed! E1");
      return false;
  }

  cmdSendInfos = new CMDSendInfo*[config->COMMANDS_LENGTH];
  for(size_t i = 0; i < config->COMMANDS_LENGTH; i++)
  {
    cmdSendInfos[i] = new CMDSendInfo();
    cmdSendInfos[i]->cmd = config->COMMANDS[i];
  }

  /* Setup SPI access */
  SPI.begin(config->CAN_SPI.PIN_SCK,
            config->CAN_SPI.PIN_MISO,
            config->CAN_SPI.PIN_MOSI,
            config->CAN_SPI.PIN_CS);

  pinMode(config->CAN_SPI.PIN_CS, OUTPUT);
  digitalWrite(config->CAN_SPI.PIN_CS, HIGH);

  /* Attach interrupt handler to register MCP2515 signaled by taking INT low */
  pinMode(config->CAN_SPI.PIN_INT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(config->CAN_SPI.PIN_INT),
                  []() IRAM_ATTR
                  {
                    self->handleInterrupt();
                  },
                  FALLING);

  mcp2515->begin();

  if(!setMode(Mode::Loopback)) // test if we can write something to the MCP2515 (is a device connected?)
  {
      SPI.end();
      mqttSerial.println("CAN-Bus init failed! E2");
      return false;
  }

  callbackCAN = [this](String label, byte *payload, unsigned int length) { handleMQTTSetRequest(label, payload, length); };

  canInited = true;

  mcp2515->setBitRate(rate); // CAN bit rate and MCP2515 clock speed

  setMode(Mode::Normal);

  mqttSerial.println("CAN-Bus inited");

  return true;
}

void DriverMCP2515::listenOnly(bool value)
{
  if(value)
    setMode(Mode::ListenOnly);
  else
    setMode(Mode::Normal);
}

void DriverMCP2515::setID(const uint16_t id)
{
  currentFrameId = id;
}

void DriverMCP2515::handleLoop()
{
  uint64_t currentMillis = millis();

  for(size_t i = 0; i < config->COMMANDS_LENGTH; i++)
  {
    if(cmdSendInfos[i]->pending == true && currentMillis - cmdSendInfos[i]->timeMessageSend >= CAN_MESSAGE_TIMEOUT * 1000)
    {
      cmdSendInfos[i]->pending = false;
      mqttSerial.printf("CAN Timeout for message: %s\n", cmdSendInfos[i]->cmd->label);
    }
  }
}

void DriverMCP2515::sendCommandWithID(CommandDef* cmd, bool setValue, int value)
{
  CanFrame frame;

  if(setValue)
  {
      frame.id = 680;
  }
  else
  {
      frame.id = cmd->id;
  }

  frame.len = sizeof(cmd->command);
  memcpy(frame.data, cmd->command, frame.len);

  if(cmd->writable && setValue)
  {
      // set first byte in command array to have HEX Value "0" on second position
      // character No 2: 0=write 1=read 2=answer
      frame.data[0] = (frame.data[0] & 0xF0) | 0x00;

      byte valByte1 = 0;
      byte valByte2 = 0;

      if(value < 0 && cmd->type != "float")
      {
          // error
          // set negative values if type not float not possible !!!
          return;
      }

      const double calculatedValue = value * cmd->divisor;

      if(cmd->type == "int")
      {
          valByte1 = calculatedValue;
      }
      else if(cmd->type == "value")
      {
          valByte1 = calculatedValue;
      }
      else if(cmd->type == "longint")
      {
          valByte1 = (int)calculatedValue >> 8;
          valByte2 = (int)calculatedValue & 0xFF;
      }
      else if(cmd->type == "float")
      {
          const int intCalcValue = (int)calculatedValue & 0xFFFF;
          valByte1 = intCalcValue >> 8;
          valByte2 = intCalcValue & 0xFF;
      }

      if (frame.data[2] == 0xFA)  // 2=pos address
      {
          // Byte 3 == FA
          // 30 0A FA 01 D6 00 D9   <- $CANMsg
          //                |  ^pos: 6
          //                ^pos: 5
          frame.data[5] = valByte1;
          frame.data[6] = valByte2;
      }
      else
      {
          // Byte 3 != FA
          // 30 0A 0E 01 E8 00 00   <- $CANMsg
          //          |  ^pos: 4
          //          ^pos: 3
          //    t_dhw - 48,8Â°
          frame.data[3] = valByte1;
          frame.data[4] = valByte2;
      }
  }
  else
  {
    for(size_t i = 0; i < config->COMMANDS_LENGTH; i++)
    {
      if(cmdSendInfos[i]->cmd == cmd)
      {
        cmdSendInfos[i]->pending = true;
        cmdSendInfos[i]->timeMessageSend = millis();
        break;
      }
    }
  }

  if(!mcp2515->transmit(frame.id, frame.data, frame.len)) {
    mqttSerial.println("ERROR TX");
  }
}