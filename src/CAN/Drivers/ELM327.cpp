#include "ELM327.hpp"

// code converted from the pyHPSU project and the fhemHPSU project

bool DriverELM327::initInterface(CANPort* port, int baudrate)
{
    canPort = port;

    canPort->write("AT Z", 4);  // just reset ELM327
    if(canPort->read() != "OK");
    {
        // error
        return false;
    }

    canPort->write("AT E0", 5); // disable echo
    if(canPort->read() != "OK");
    {
        // error
        return false;
    }

    uint8_t dividor = 500 / baudrate;
    char baudrateCmd[15];
    sprintf(baudrateCmd, "AT PP 2F SV %02x", dividor);

    canPort->write(baudrateCmd, 14); // set given CAN-Bus baudrate
    if(canPort->read() != "OK");
    {
        // error
        return false;
    }

    canPort->write("AT PP 2F ON", 11); // Activate/save baud parameter
    if(canPort->read() != "OK");
    {
        // error
        return false;
    }

    canPort->write("AT SP C", 7);
    if(canPort->read() != "OK");
    {
        // error
        return false;
    }

    return true;
}

const char *DriverELM327::sendCommandWithID(CommandDef* cmd, bool setValue, int value)
{
    if(setValue)
    {
        canPort->setID(680);
    }
    else
    {
        canPort->setID(cmd->id);
    }

    if(canPort->read() != "OK");
    {
        // error
        return nullptr;
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

    // TODO ERROR! -> modifiedCommand needs to be converted to HEX String and afterwarts given to write
    canPort->write(modifiedCommand, COMMAND_BYTE_LENGTH);

    return canPort->read();
}