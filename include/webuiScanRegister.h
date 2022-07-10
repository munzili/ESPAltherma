#ifndef WEBUI_SCAN_REGISTER_H
#define WEBUI_SCAN_REGISTER_H
#include <Arduino.h>
#include "X10A.h"
#include "ArduinoJson.h"
#include "labeldef.h"
#include "arrayFunctions.h"
#include "comm.h"
#include "converters.h"
#include "config.h"
#include "mqttSerial.h"

#define MODELS_DOC_SIZE 1024*10

struct WebUIScanRegister {
  int8_t PinRx;
  int8_t PinTx;
  String Params;
};

enum ValueLoadState {
  NotLoading,
  Pending,
  Loading,
  LoadingFinished
};

WebUIScanRegister webuiScanRegisterConfig;
ValueLoadState valueLoadState = NotLoading;
String valueLoadResponse;

void webuiScanRegister()
{
  if(valueLoadState == NotLoading || valueLoadState == LoadingFinished)
  {
    return;
  }

  valueLoadState = Loading;
  
  bool serialX10AWasInited = SerialX10A;

  Serial.printf("Starting new serial connection with pins RX: %u, TX: %u\n", webuiScanRegisterConfig.PinRx, webuiScanRegisterConfig.PinTx);
  Serial.println("Waiting for registry scan to finish...");

  Serial.println("Starting registry scan...");

  X10AInit(webuiScanRegisterConfig.PinRx, webuiScanRegisterConfig.PinTx);

  DynamicJsonDocument modelsDoc(MODELS_DOC_SIZE);
  deserializeJson(modelsDoc, webuiScanRegisterConfig.Params); 
  JsonArray modelsDocArr = modelsDoc.as<JsonArray>();

  Serial.printf("Creating labelDefs %i\n", modelsDocArr.size());

  size_t labelsSize = modelsDocArr.size();
  LabelDef **labelsToLoad = new LabelDef*[labelsSize];
  
  uint8_t counter = 0;
  for (JsonArray model : modelsDocArr) 
  {
    labelsToLoad[counter] = new LabelDef(model[0], model[1], model[2], model[3], model[4], model[5]);
    counter++;
  }  

  //getting the list of registries to query from the selected values  
  uint8_t loadRegistryBufferSize = 0;  
  uint8_t* tempRegistryIDs = new uint8_t[labelsSize];

  size_t i;
  for (i = 0; i < labelsSize; i++)
  {            
    auto &&label = *labelsToLoad[i];

    if (!contains(tempRegistryIDs, labelsSize, label.registryID))
    {
      Serial.printf("Adding registry 0x%02x to be queried.\n", label.registryID);
      tempRegistryIDs[loadRegistryBufferSize++] = label.registryID;
    }
  }
  
  RegistryBuffer loadRegistryBuffers[loadRegistryBufferSize];

  for(i = 0; i < loadRegistryBufferSize; i++)
  {
    loadRegistryBuffers[i].RegistryID = tempRegistryIDs[i];
  }

  delete[] tempRegistryIDs;

  if (loadRegistryBufferSize == 0)
  {
    valueLoadState = LoadingFinished;
    mqttSerial.println("Given params doesn't contain a registry buffer to fetch");
    return;
  }

  Serial.println("Fetching Values");
  
  //Querying all registries and store results
  for (size_t i = 0; i < loadRegistryBufferSize; i++)
  {
    uint8_t tries = 0;
    while (tries++ < 3 && !queryRegistry(&loadRegistryBuffers[i]))
    {
      Serial.println("Retrying...");
      delay(1000);
    }
  }
  
  for (size_t i = 0; i < labelsSize; i++)
  {            
    auto &&label = *labelsToLoad[i];

    for (size_t j = 0; j < loadRegistryBufferSize; j++)
    {
      if(loadRegistryBuffers[j].Success && label.registryID == loadRegistryBuffers[j].RegistryID)
      {
        char *input = loadRegistryBuffers[j].Buffer;
        input += label.offset + 3;

        converter.convert(&label, input); // convert buffer result of label offset to correct/usabel value
        break;
      }
    }
  }  
  
  Serial.println("Returning Values");

  DynamicJsonDocument resultDoc(labelsSize*JSON_OBJECT_SIZE(2));
  JsonArray obj = resultDoc.to<JsonArray>();
  
  for (uint8_t i = 0; i < labelsSize; i++) {
    obj.add(labelsToLoad[i]->asString);
  } 

  for (size_t i = 0; i < labelsSize; i++)
  {
      delete labelsToLoad[i];
  }
  delete[] labelsToLoad;

  if(serialX10AWasInited)
  {
    Serial.println("Restoring original X10A connection");
    X10AInit(config->PIN_RX, config->PIN_TX);
  }
  else
  {
    X10AEnd();
  }

  Serial.println("Finished registry scan");
  
  serializeJson(resultDoc, valueLoadResponse);

  valueLoadState = LoadingFinished;
}

#endif