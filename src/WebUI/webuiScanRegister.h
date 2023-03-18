#ifndef WEBUI_SCAN_REGISTER_H
#define WEBUI_SCAN_REGISTER_H

#include <Arduino.h>
#include "X10A/X10A.h"
#include "ArduinoJson.h"
#include "Config/parameterDef.h"
#include "arrayFunctions.h"
#include "X10A/comm.h"
#include "X10a/converters.h"
#include "Config/config.h"
#include "MQTT/mqttSerial.h"

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

extern WebUIScanRegister webuiScanRegisterConfig;
extern ValueLoadState valueLoadState;
extern String valueLoadResponse;

void webuiScanRegister();

#endif