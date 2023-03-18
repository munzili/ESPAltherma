#ifndef X10A_H
#define X10A_H

#include <HardwareSerial.h>

extern HardwareSerial SerialX10A;

void X10AEnd();

void X10AInit(int8_t rxPin, int8_t txPin);

#endif