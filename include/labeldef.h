#ifndef LABELDEF_H
#define LABELDEF_H
#include <pgmspace.h>

struct LabelDef
{
    const uint8_t registryID;    
    const uint8_t offset;
    const uint16_t convid;
    const int8_t dataSize;
    const int8_t dataType;
    const String label;
    char asString[30];
    LabelDef(int registryIDp, int offsetp, int convidp, int dataSizep, int dataTypep, String labelp) : registryID(registryIDp), offset(offsetp), convid(convidp), dataSize(dataSizep), dataType(dataTypep), label(labelp){};
};

#endif