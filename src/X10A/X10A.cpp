#include "X10A.hpp"

HardwareSerial SerialX10A(1);

size_t registryBufferSize;
RegistryBuffer *registryBuffers; //Holds the registries to query and the last returned data

void X10AEnd()
{
  if(SerialX10A)
  {
    SerialX10A.end();
  }
}

void initRegistries()
{
  //getting the list of registries to query from the selected values
  registryBufferSize = 0;
  uint8_t* tempRegistryIDs = new uint8_t[config->PARAMETERS_LENGTH];

  size_t i;
  for (i = 0; i < config->PARAMETERS_LENGTH; i++)
  {
    auto &&label = *config->PARAMETERS[i];

    if (!contains(tempRegistryIDs, config->PARAMETERS_LENGTH, label.registryID))
    {
      mqttSerial.printf("Adding registry 0x%2x to be queried.\n", label.registryID);
      tempRegistryIDs[registryBufferSize++] = label.registryID;
    }
  }

  registryBuffers = new RegistryBuffer[registryBufferSize];

  for(i = 0; i < registryBufferSize; i++)
  {
    registryBuffers[i].RegistryID = tempRegistryIDs[i];
  }

  delete[] tempRegistryIDs;
}

void handleX10A()
{
  //Querying all registries and store results
  for (size_t i = 0; i < registryBufferSize; i++)
  {
    uint8_t tries = 0;
    while (tries++ < 3 && !queryRegistry(&registryBuffers[i]))
    {
      mqttSerial.println("Retrying...");
      waitLoop(1000);
    }
  }

  for (size_t i = 0; i < config->PARAMETERS_LENGTH; i++)
  {
    auto &&label = *config->PARAMETERS[i];

    for (size_t j = 0; j < registryBufferSize; j++)
    {
      if(registryBuffers[j].Success && label.registryID == registryBuffers[j].RegistryID)
      {
        char *input = registryBuffers[j].Buffer;
        input += label.offset + 3;

        converter.convert(&label, input); // convert buffer result of label offset to correct/usabel value

        updateValues(&label);       //send them in mqtt
        waitLoop(500);//wait .5sec between registries
        break;
      }
    }
  }

  sendValues();//Send the full json message
}

void X10AInit(int8_t rxPin, int8_t txPin)
{
  X10AEnd();
  SerialX10A.begin(9600, SERIAL_8E1, rxPin, txPin);
}