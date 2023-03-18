#ifndef CAN_PORT_H
#define CAN_PORT_H

class CANPort
{
public:
    virtual void write(const char* bytes);
    virtual const char* read();
};

#endif