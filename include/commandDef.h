#ifndef COMMANDDEF_H
#define COMMANDDEF_H
#include <pgmspace.h>
#include <string.h>

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
    CommandDefValueCode(String keyp, String valuep) : key(keyp), value(valuep){};
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

    CommandDef( const char *labelp, byte commandp[COMMAND_BYTE_LENGTH], uint16_t idp, float divisorp, bool writablep, const char *unitp, const char *typep, const uint8_t valueCodeSizep, CommandDefValueCode** valueCodep) : id(idp), divisor(divisorp), writable(writablep), valueCodeSize(valueCodeSizep)
    {
        uint8_t i;

        for(i = 0; i < COMMAND_BYTE_LENGTH; i++)
            command[i] = commandp[i];

        strcpy(label, labelp);
        strcpy(unit, unitp);
        strcpy(type, typep);

        valueCode = valueCodep;
    };
};

#endif