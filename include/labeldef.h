#ifndef labeldef_h
#define labeldef_h
#include <pgmspace.h>
#define LABELDEF

class LabelDef
{
public:
    int registryID;    
    int offset;
    int convid;
    int dataSize;
    int dataType;
    const char *label;
    char asString[30];
    LabelDef(){};
    LabelDef(int registryIDp, int offsetp, int convidp, int dataSizep, int dataTypep, const char *labelp) : registryID(registryIDp), offset(offsetp), convid(convidp), dataSize(dataSizep), dataType(dataTypep), label(labelp){};
};

// TODO: FIX AND REMOVE!!
LabelDef** labelDefs;
uint8_t labelDefsSize = 0;
#endif