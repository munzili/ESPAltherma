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

void initRegistries(RegistryBuffer** buffer, size_t& bufferSize, ParameterDef** parameters, size_t parametersLength)
{
  //getting the list of registries to query from the selected values
  bufferSize = 0;
  uint8_t* tempRegistryIDs = new uint8_t[parametersLength]();

  size_t i;
  for (i = 0; i < parametersLength; i++)
  {
    auto &&label = *parameters[i];

    if (!contains(tempRegistryIDs, parametersLength, label.registryID))
    {
      mqttSerial.printf("Adding registry 0x%2x to be queried.\n", label.registryID);
      tempRegistryIDs[bufferSize++] = label.registryID;
    }
  }

  *buffer = new RegistryBuffer[bufferSize];

  for(i = 0; i < bufferSize; i++)
  {
    (*buffer)[i].RegistryID = tempRegistryIDs[i];
  }

  delete[] tempRegistryIDs;
}

void handleX10A(RegistryBuffer* buffer, size_t& bufferSize, ParameterDef** parameters, size_t parametersLength, bool sendValuesViaMQTT)
{
  //Querying all registries and store results
  for (size_t i = 0; i < bufferSize; i++)
  {
    uint8_t tries = 0;
    while (tries++ < 3 && !queryRegistry(&buffer[i]))
    {
      mqttSerial.println("Retrying...");
      waitLoop(1000);
    }
  }

  for (size_t i = 0; i < parametersLength; i++)
  {
    auto &&label = *parameters[i];

    for (size_t j = 0; j < bufferSize; j++)
    {
      if(buffer[j].Success && label.registryID == buffer[j].RegistryID)
      {
        char *input = buffer[j].Buffer;
        input += label.offset + 3;

        converter.convert(&label, input); // convert buffer result of label offset to correct/usabel value

        if(sendValuesViaMQTT)
        {
          updateValues(&label);       //send them in mqtt
          waitLoop(500);//wait .5sec between registries
        }

        break;
      }
    }
  }

  if(sendValuesViaMQTT)
  {
    sendValues();//Send the full json message
  }
}

void X10AInit(int8_t rxPin, int8_t txPin)
{
  X10AEnd();
  SerialX10A.begin(9600, SERIAL_8E1, rxPin, txPin);
}