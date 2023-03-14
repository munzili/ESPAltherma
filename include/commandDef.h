#ifndef COMMANDDEF_H
#define COMMANDDEF_H
#include <pgmspace.h>
#include <string.h>

#define COMMAND_BYTE_LENGTH 7
#define COMMAND_UNIT_LENGTH 7
#define COMMAND_TYPE_LENGTH 7

struct CommandDef
{
    const String label; // maybe char[20] ?
    byte command[COMMAND_BYTE_LENGTH];
    /*const uint16_t id;
    const float divisor;
    const bool writable;
    char unit[COMMAND_UNIT_LENGTH];
    char type[COMMAND_TYPE_LENGTH];
    const String valueCode;
    char asString[32] = "";*/

    CommandDef(String labelp, byte commandp[COMMAND_BYTE_LENGTH]/*, uint16_t idp, float divisorp, bool writablep, char *unitp, char *typep, String valueCodep*/) : label(labelp)/*, id(idp), divisor(divisorp), writable(writablep), valueCode(valueCodep)*/
    {
        uint8_t i;

        for(i = 0; i < COMMAND_BYTE_LENGTH; i++)
            command[i] = commandp[i];

        /*for(i = 0; i < COMMAND_UNIT_LENGTH; i++)
            unit[i] = unitp[i];

        for(i = 0; i < COMMAND_TYPE_LENGTH; i++)
            type[i] = typep[i];*/
    };
};

#endif