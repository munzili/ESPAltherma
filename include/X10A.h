#ifndef X10A_H
#define X10A_H

#include <HardwareSerial.h>

HardwareSerial SerialX10A(1);

void X10AInit(int8_t rxPin, int8_t txPin)
{
  if(SerialX10A)
  {
    SerialX10A.end();
  }

  SerialX10A.begin(9600, SERIAL_8E1, rxPin, txPin);
}

#endif