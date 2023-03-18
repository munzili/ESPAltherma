#include "X10A.h"

HardwareSerial SerialX10A(1);

void X10AEnd()
{
  if(SerialX10A)
  {
    SerialX10A.end();
  }
}

void X10AInit(int8_t rxPin, int8_t txPin)
{
  X10AEnd();
  SerialX10A.begin(9600, SERIAL_8E1, rxPin, txPin);
}