#ifndef COMMANDDEF_H
#define COMMANDDEF_H

#include <pgmspace.h>
#include "ArduinoC.hpp"

#define COMMAND_LABEL_LENGTH 21
#define COMMAND_BYTE_LENGTH 7
#define COMMAND_UNIT_LENGTH 8
#define COMMAND_TYPE_LENGTH 8

#define COMMANDDEF_INDEX_LABEL 0
#define COMMANDDEF_INDEX_COMMAND 1
#define COMMANDDEF_INDEX_ID 2
#define COMMANDDEF_INDEX_DIVISOR 3
#define COMMANDDEF_INDEX_WRITABLE 4
#define COMMANDDEF_INDEX_UNIT 5
#define COMMANDDEF_INDEX_TYPE 6
#define COMMANDDEF_INDEX_VALUE_CODE 7

struct CommandDefValueCode
{
    const String key;
    const String value;
    CommandDefValueCode(String keyp, String valuep);
};

struct CommandDef
{
    char label[COMMAND_LABEL_LENGTH];
    byte command[COMMAND_BYTE_LENGTH];
    const uint16_t id;
    const float divisor;
    const bool writable;
    char unit[COMMAND_UNIT_LENGTH];
    char type[COMMAND_TYPE_LENGTH];
    const uint8_t valueCodeSize;
    CommandDefValueCode** valueCode;

    CommandDef( const char *labelp, byte commandp[COMMAND_BYTE_LENGTH], uint16_t idp, float divisorp, bool writablep, const char *unitp, const char *typep, const uint8_t valueCodeSizep, CommandDefValueCode** valueCodep);
};

#endif