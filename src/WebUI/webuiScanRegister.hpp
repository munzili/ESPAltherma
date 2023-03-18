#ifndef WEBUI_SCAN_REGISTER_H
#define WEBUI_SCAN_REGISTER_H

#include "ArduinoC.hpp"
#include "X10A/X10A.hpp"
#include "ArduinoJson.hpp"
#include "Config/parameterDef.hpp"
#include "arrayFunctions.hpp"
#include "X10A/comm.hpp"
#include "X10a/converters.hpp"
#include "Config/config.hpp"
#include "MQTT/mqttSerial.hpp"

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