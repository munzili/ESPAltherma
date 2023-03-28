#include "SJA1000.hpp"

bool DriverSJA1000::initInterface(CANPort* port, int baudrate)
{
    canPort = port;

    //CAN.begin(baudrate * 1000);

    return true;
}

const char* DriverSJA1000::sendCommandWithID(CommandDef* cmd, bool setValue, int value)
{
    if(setValue)
    {
        canPort->setID(680);
    }
    else
    {
        canPort->setID(cmd->id);
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
            return nullptr;
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

    canPort->write(modifiedCommand, COMMAND_BYTE_LENGTH);
    if(canPort->read() != "OK");
    {
        // error
        return nullptr;
    }

    return nullptr;
}