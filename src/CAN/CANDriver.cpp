#include "CANDriver.hpp"

CanFrame* CANDriver::getCanFrameFromCommand(CommandDef* cmd, bool setValue, int value)
{
  CanFrame* frame = new CanFrame();

  if(setValue)
  {
      frame->id = 680;
  }
  else
  {
      frame->id = cmd->id;
  }

  frame->len = sizeof(cmd->command);
  memcpy(frame->data, cmd->command, frame->len);

  if(cmd->writable && setValue)
  {
      // set first byte in command array to have HEX Value "0" on second position
      // character No 2: 0=write 1=read 2=answer
      frame->data[0] = (frame->data[0] & 0xF0) | 0x00;

      byte valByte1 = 0;
      byte valByte2 = 0;

      if(value < 0 && strcmp(cmd->type, "float") != 0)
      {
          // error
          // set negative values if type not float not possible !!!
          delete frame;
          return nullptr;
      }

      const double calculatedValue = value * cmd->divisor;

      if(strcmp(cmd->type, "int") == 0)
      {
          valByte1 = calculatedValue;
      }
      else if(strcmp(cmd->type, "value") == 0)
      {
          valByte1 = calculatedValue;
      }
      else if(strcmp(cmd->type, "longint") == 0)
      {
          valByte1 = (int)calculatedValue >> 8;
          valByte2 = (int)calculatedValue & 0xFF;
      }
      else if(strcmp(cmd->type, "float") == 0)
      {
          const int intCalcValue = (int)calculatedValue & 0xFFFF;
          valByte1 = intCalcValue >> 8;
          valByte2 = intCalcValue & 0xFF;
      }

      if (frame->data[2] == 0xFA)  // 2=pos address
      {
          // Byte 3 == FA
          // 30 0A FA 01 D6 00 D9   <- $CANMsg
          //                |  ^pos: 6
          //                ^pos: 5
          frame->data[5] = valByte1;
          frame->data[6] = valByte2;
      }
      else
      {
          // Byte 3 != FA
          // 30 0A 0E 01 E8 00 00   <- $CANMsg
          //          |  ^pos: 4
          //          ^pos: 3
          //    t_dhw - 48,8Â°
          frame->data[3] = valByte1;
          frame->data[4] = valByte2;
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

  mqttSerial.printf("CAN: Transmiting ID(%i) ", frame->id);
  for(uint8_t i = 0; i < frame->len; i++)
  {
    mqttSerial.printf("%02x ", frame->data[i]);
  }
  mqttSerial.println();

  return frame;
}

void CANDriver::sniffCAN(const uint32_t timestamp_us, CanFrame const frame)
{
  char resultText[128] = "";
  sprintf(resultText, "CAN [ %i ] ID", timestamp_us);

  if(frame.isRTR) strcat(resultText, "(RTR)");
  if(frame.isEXT) strcat(resultText, "(EXT)");

  sprintf(resultText + strlen(resultText), " %02X DATA[%i] ", frame.id, frame.len);

  std::for_each(frame.data,
                frame.data + frame.len,
                [resultText](uint8_t const elem) mutable {
                    sprintf(resultText + strlen(resultText), "%02X ", elem);
                });

  mqttSerial.println(resultText);
}

void CANDriver::handleLoop()
{
  if(!canInited)
      return;

  uint64_t currentMillis = millis();

  for(size_t i = 0; i < config->COMMANDS_LENGTH; i++)
  {
    if(cmdSendInfos[i]->pending == true && currentMillis - cmdSendInfos[i]->timeMessageSend >= CAN_MESSAGE_TIMEOUT * 1000)
    {
      cmdSendInfos[i]->pending = false;
      mqttSerial.printf("CAN Timeout for message: %s\n", cmdSendInfos[i]->cmd->label);
    }
  }

  if(config->CAN_AUTOPOLL_MODE == CANPollMode::Auto)
  {
    ulong currentTime = millis();

    if(currentTime - lastTimeRunned >= config->CAN_AUTOPOLL_TIME * 1000)
    {
      mqttSerial.printf("CAN Poll Mode Auto Reading: %lu\n", currentTime);

      for(size_t i = 0; i < config->COMMANDS_LENGTH; i++)
      {
        if(cmdSendInfos[i]->pending == false)
        {
          sendCommand(config->COMMANDS[i], false);
        }
      }

      lastTimeRunned = currentTime;
    }
  }
}

void CANDriver::onDataRecieved(uint32_t const timestamp_us, CanFrame const frame)
{
  if(!canInited)
      return;

  if(sniffingEnabled || currentMode == CanDriverMode::Loopback)
  {
    sniffCAN(timestamp_us, frame);

    if(currentMode == CanDriverMode::Loopback)
      return;
  }

  if(frame.len < 2)
    return;

  bool extended = frame.data[2] == 0xFA;

  bool valid = false;
  CommandDef* recievedCommand = getCommandFromData(frame.data);

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

    valByte1 = frame.data[5];
    valByte2 = frame.data[6];
  }
  else
  {
    //20 0A 0E 01 E8 00 00   <- $CANMsg
    //         |  ^pos: 4
    //         ^pos: 3
    //   t_dhw - 48,8Â°
    valByte1 = frame.data[3];
    valByte2 = frame.data[4];
  }

  int value;

  if(strcmp(recievedCommand->type, "int") == 0)
  {
    value = HPSU_toSigned(valByte1, recievedCommand->unit);
  }
  else if(strcmp(recievedCommand->type, "value") == 0)
  {
    value = valByte1;
    //example: mode_01 val 4       -> 31 00 FA 01 12 04 00
  }
  else if(strcmp(recievedCommand->type, "longint") == 0)
  {
    value = HPSU_toSigned(valByte2 + valByte1 * 0x0100, recievedCommand->unit);
    //example: one_hot_water val 1 -> 31 00 FA 01 44 00 01
    //                                                   ^
  }
  else if(strcmp(recievedCommand->type, "float") == 0)
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

  mqttSerial.printf("CAN Data recieved %s: %s\n", recievedCommand->label, valueCodeKey.c_str());
}


CommandDef* CANDriver::getCommandFromData(const uint8_t *data)
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

void CANDriver::listenOnly(bool value)
{
  if(value)
    setMode(CanDriverMode::ListenOnly);
  else
    setMode(CanDriverMode::Normal);
}

int CANDriver::HPSU_toSigned(uint16_t value, char* unit)
{
  if(strcmp(unit, "deg") == 0 || strcmp(unit, "value_code_signed") == 0)
  {
    int newValue = value & 0xFFFF;
    return (newValue ^ 0x8000) - 0x8000;
  }
  else
  {
    return value;
  }
}

void CANDriver::enableSniffing(bool value)
{
  sniffingEnabled = value;
}

void CANDriver::handleMQTTSetRequest(const String &label, const char *payload, const uint32_t length)
{
  if(!canInited)
      return;

  const int payloadAsInt = atoi(payload);

  for(size_t i = 0; i < config->COMMANDS_LENGTH; i++)
  {
    if(config->COMMANDS[i]->writable && strcmp(config->COMMANDS[i]->name, label.c_str()) == 0)
    {
      mqttSerial.printf("CAN: Got MQTT SET request for %s, %08x\n", label, payloadAsInt);
      sendCommand(config->COMMANDS[i], true, payloadAsInt);
      return;
    }
  }

  mqttSerial.printf("CAN: Got invalid MQTT SET request for %s\n", label);
}

void CANDriver::defaultInit()
{
  cmdSendInfos = new CMDSendInfo*[config->COMMANDS_LENGTH];
  for(size_t i = 0; i < config->COMMANDS_LENGTH; i++)
  {
    cmdSendInfos[i] = new CMDSendInfo();
    cmdSendInfos[i]->cmd = config->COMMANDS[i];
  }

  callbackCAN = [this](const String label, const char *payload, const uint32_t length) { handleMQTTSetRequest(label, payload, length); };

  canInited = true;

  setMode(CanDriverMode::Normal);

  mqttSerial.println("CAN-Bus inited");
}
