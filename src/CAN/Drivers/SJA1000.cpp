#include "SJA1000.hpp"

bool DriverSJA1000::initInterface()
{
    // Konfiguration des CAN-Controllers
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)config->PIN_CAN_TX, (gpio_num_t)config->PIN_CAN_RX, TWAI_MODE_NORMAL);
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    twai_timing_config_t t_config;

    switch (config->CAN_SPEED_KBPS)
    {
#if (SOC_TWAI_BRP_MAX > 256)
    case 10:
        t_config = TWAI_TIMING_CONFIG_10KBITS();
        break;
#endif
#if (SOC_TWAI_BRP_MAX > 128) || (CONFIG_ESP32_REV_MIN >= 2)
    case 20:
        t_config = TWAI_TIMING_CONFIG_20KBITS();
        break;
#endif

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
        return false; // error - wrong speed
        break;
    }

    int result;

    // Initialisierung des CAN-Controllers
    if ((result = twai_driver_install(&g_config, &t_config, &f_config)) != ESP_OK)
    {
        mqttSerial.print("CAN-Bus init failed! E2 - ");
        mqttSerial.println(result);
        return false;
    }

    if ((result = twai_start()) != ESP_OK)
    {
        mqttSerial.print("CAN-Bus init failed! E3 - ");
        mqttSerial.println(result);
        return false;
    }

    return true;
}

void DriverSJA1000::sendCommand(CommandDef* cmd, bool setValue, int value)
{
    if(setValue)
    {
        //canPort->setID(680);
    }
    else
    {
        //canPort->setID(cmd->id);
    }

    byte modifiedCommand[COMMAND_BYTE_LENGTH];
    memcpy(modifiedCommand, cmd->command, COMMAND_BYTE_LENGTH);

    if(cmd->writable && setValue)
    {
        // set first byte in command array to have HEX Value "0" on second position
        // character No 2: 0=write 1=read 2=answer
        modifiedCommand[0] = (modifiedCommand[0] & 0xF0) | 0x00;

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

        if (modifiedCommand[2] == 0xFA)  // 2=pos address
        {
            // Byte 3 == FA
            // 30 0A FA 01 D6 00 D9   <- $CANMsg
            //                |  ^pos: 6
            //                ^pos: 5
            modifiedCommand[5] = valByte1;
            modifiedCommand[6] = valByte2;
        }
        else
        {
            // Byte 3 != FA
            // 30 0A 0E 01 E8 00 00   <- $CANMsg
            //          |  ^pos: 4
            //          ^pos: 3
            //    t_dhw - 48,8Â°
            modifiedCommand[3] = valByte1;
            modifiedCommand[4] = valByte2;
        }
    }

    /*canPort->write(modifiedCommand, COMMAND_BYTE_LENGTH);
    if(canPort->read() != "OK");
    {
        // error
        return nullptr;
    }*/
}

void readAllMessages()
{
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
}