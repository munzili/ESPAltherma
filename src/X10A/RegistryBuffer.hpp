#ifndef REGISTRY_BUFFER_H
#define REGISTRY_BUFFER_H

#define MAX_BUFFER_SIZE 32 //max bytes returned from X10A Port

struct RegistryBuffer {
  char RegistryID;
  char Buffer[MAX_BUFFER_SIZE];
  bool Success;
  char CRC;
};

#endif