#ifndef HPSU_H
#define HPSU_H

#include <CAN.h>
#include "Config/config.hpp"
#include "Config/commandDef.hpp"

#define UM_DEGREE "deg"
#define UM_BOOLEAN "bool"
#define UM_PERCENT "percent"
#define UM_INT "int"
#define UM_BAR "bar"
#define UM_HOUR "hour"

#define CANID_SET_VALUE 680

class HPSU
{
private:

public:
    HPSU();
    char* sendCommandWithParse(CommandDef cmd, bool setValue = false);
    bool sendCommand(CommandDef cmd, bool setValue);
    char* parseCommand(CommandDef cmd, char* response);
};

#endif