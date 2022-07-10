#ifndef COMM_H
#define COMM_H
#include <Arduino.h>
#include <HardwareSerial.h>
#include "X10A.h"

#define SER_TIMEOUT 300 //leave 300ms for the machine to answer
#define MAX_BUFFER_SIZE 32 //max bytes returned from X10A Port

struct RegistryBuffer {
  char RegistryID;
  char Buffer[MAX_BUFFER_SIZE];
  bool Success;
  char CRC;
};

char getCRC(char *src, int len)
{
  char b = 0;
  for (int i = 0; i < len; i++)
  {
    b += src[i];
  }
  return ~b;
}

bool queryRegistry(RegistryBuffer *registryBuffer)
{
  // clear buffer first
  memset(registryBuffer->Buffer, 0, sizeof(registryBuffer->Buffer));
  registryBuffer->CRC = 0;
  registryBuffer->Success = false;

  //preparing command:
  char prep[] = {0x03, 0x40, registryBuffer->RegistryID, 0x00};
  prep[3] = getCRC(prep, 3);

  //Sending command to serial
  SerialX10A.flush(); //Prevent possible pending info on the read
  SerialX10A.write(prep, 4);
  ulong start = millis();

  int len = 0;
  registryBuffer->Buffer[2] = 10;
  mqttSerial.printf("Querying register 0x%02x... ", registryBuffer->RegistryID);
  while ((len < registryBuffer->Buffer[2] + 2) && (millis() < (start + SER_TIMEOUT)))
  {
    if (SerialX10A.available())
    {
      registryBuffer->Buffer[len++] = SerialX10A.read();
    }
  }
  if (millis() >= (start + SER_TIMEOUT))
  {
    if (len == 0)
    {
      mqttSerial.printf("Time out! Check connection\n");
    }
    else
    {
      mqttSerial.printf("ERR: Time out on register 0x%02x! got %d/%d bytes\n", registryBuffer->RegistryID, len, registryBuffer->Buffer[2]);
      char bufflog[MAX_BUFFER_SIZE * 5] = {0};
      for (size_t i = 0; i < len; i++)
      {
        sprintf(bufflog + i * 5, "0x%02x ", registryBuffer->Buffer[i]);
      }
      mqttSerial.print(bufflog);
    }
    delay(500);
    return false;
  }

  registryBuffer->CRC = getCRC(registryBuffer->Buffer, len - 1);

  if (registryBuffer->CRC != registryBuffer->Buffer[len - 1])
  {
    Serial.println("Wrong CRC!");
    mqttSerial.printf("ERROR: Wrong CRC on register 0x%02x!", registryBuffer->RegistryID);
    Serial.printf("Calculated 0x%2x but got 0x%2x\n", registryBuffer->CRC, registryBuffer->Buffer[len - 1]);
    return false;
  }
  else
  {
    Serial.println(".. CRC OK!");
    registryBuffer->Success = true;
    return true;
  }
}
#endif