#ifndef WEBUI_H
#define WEBUI_H
#include <LittleFS.h>
#include <Update.h>
#include "ESPAsyncWebServer.h" 
#include "ArduinoJson.h"
#include "comm.h"
#include "esp_task_wdt.h"
#include "config.h"
#include "wireless.h"
#include "persistence.h"
#include "arrayFunctions.h"
#include "webuiScanRegister.h"

#define MODELS_FILE "/models.json"
#define MODEL_DEFINITION_DOC_SIZE 1024*25
#define MODEL_DEFINITION_UOLOAD_SIZE 1024*50
#define MODELS_DOC_SIZE 1024*10
#define WEBUI_SELECTION_VALUE_SIZE 1024

// Set web server port number to 80
AsyncWebServer server(80);

extern const uint8_t mainJS_start[] asm("_binary_webui_main_js_gz_start");
extern const uint8_t mainJS_end[] asm("_binary_webui_main_js_gz_end");
extern const uint8_t md5JS_start[] asm("_binary_webui_md5_min_js_gz_start");
extern const uint8_t md5JS_end[] asm("_binary_webui_md5_min_js_gz_end");
extern const uint8_t indexHTML_start[] asm("_binary_webui_index_html_gz_start");
extern const uint8_t indexHTML_end[] asm("_binary_webui_index_html_gz_end");
extern const uint8_t picoCSS_start[] asm("_binary_webui_pico_min_css_gz_start");
extern const uint8_t picoCSS_end[] asm("_binary_webui_pico_min_css_gz_end");
extern const uint8_t mainCSS_start[] asm("_binary_webui_main_css_gz_start");
extern const uint8_t mainCSS_end[] asm("_binary_webui_main_css_gz_end");

String lastUploadFileName;
bool webOTAIsBusy = false;

bool formatDefaultFS()
{
  LittleFS.end();
  bool result = LittleFS.format();

  if(!result)
    return false;

  LittleFS.begin();
  File file = LittleFS.open(MODELS_FILE, FILE_WRITE, true);
  file.print("[]");
  file.close();

  resetPersistence();

  return true;
}

void onLoadWifiNetworks(AsyncWebServerRequest *request)
{  
  DynamicJsonDocument networksDoc(MODELS_DOC_SIZE);

  scan_wifi();

  for (int16_t i = 0; i < lastWifiScanResultAmount; i++) 
  {
    JsonObject networkDetails = networksDoc.createNestedObject();
    networkDetails["SSID"] = lastWifiScanResults[i]->SSID;
    networkDetails["RSSI"] = lastWifiScanResults[i]->RSSI;
    networkDetails["EncryptionType"] = (lastWifiScanResults[i]->EncryptionType == WIFI_AUTH_OPEN) ? "":"*"; 
    delay(10);
  }

  scan_wifi_delete_result();

  String response;
  serializeJson(networksDoc, response);

  request->send(200, "application/json", response);
}

void onLoadBoardInfo(AsyncWebServerRequest *request)
{
#if defined(ARDUINO_M5Stick_C)
  const String response = 
    "{"
      "\"Pins\": {"
        "\"0\": \"GPIO0 - ADC2_1,TOUCH1\","
#if defined(ARDUINO_M5Stick_C_Plus)
        "\"25\": \"GPIO25 - ADC2_8,DAC_1\","      
#endif
        "\"26\": \"GPIO26 - ADC2_9,DAC_2\","
        "\"32\": \"GPIO32 - ADC1_4,TOUCH9,XTAL32\","
        "\"33\": \"GPIO33 - ADC1_5,TOUCH8,XTAL32\","
        "\"36\": \"GPIO36 - ADC1_0,SensVP\","
        "\"37\": \"GPIO37 - BTN\","
        "\"39\": \"GPIO39 - BTN\""
      "},"
      "\"Default\": {"
        "\"pin_rx\": 36,"
        "\"pin_tx\": 26,"
        "\"pin_therm\": 0,"
        "\"pin_sg1\": 32,"
        "\"pin_sg2\": 33,"
        "\"pin_enable_config\": 39,"
        "\"frequency\": 30000,"
        "\"mqtt_onetopic\": \"espaltherma/OneATTR/\","
        "\"mqtt_port\": 1883"
      "}"
    "}";
#elif defined(ESP32)
  const String response = 
    "{"
      "\"Pins\": {"
        "\"0\": \"GPIO0 - ADC2_1,TOUCH1\","
        "\"1\": \"GPIO1 - U0_TXD,CLK3\","
        "\"2\": \"GPIO2 - ADC2_2,TOUCH2,CS\","
        "\"3\": \"GPIO3 - U0_RXD,CLK2\","
        "\"4\": \"GPIO4 - ADC2_0,TOUCH0\","
        "\"5\": \"GPIO5 - VSPI_CS\","       
        "\"12\": \"GPIO12 - ADC2_5,TOUCH5,HSPI_MISO\","
        "\"13\": \"GPIO13 - ADC2_4,TOUCH4,HSPI_MOSI\","
        "\"14\": \"GPIO14 - ADC2_6,TOUCH6,HSPI_CLK\","
        "\"15\": \"GPIO15 - ADC2_3,TOUCH3,HSPI_CS\","
        "\"16\": \"GPIO16 - U2_RXD\","
        "\"17\": \"GPIO17 - U2_TXD\","
        "\"18\": \"GPIO18 - SCK,VSPI_CLK\","
        "\"19\": \"GPIO19 - MISO,VSPI_MISO\","
        "\"21\": \"GPIO21 - SDA\","
        "\"22\": \"GPIO22 - SCL\","
        "\"23\": \"GPIO23 - MOSI,VSPI_MOSI\","
        "\"25\": \"GPIO25 - ADC2_8,DAC_1\","
        "\"26\": \"GPIO26 - ADC2_9,DAC_2\","
        "\"27\": \"GPIO27 - ADC2_7,TOUCH7\","
        "\"32\": \"GPIO32 - ADC1_4,TOUCH9,XTAL32\","
        "\"33\": \"GPIO33 - ADC1_5,TOUCH8,XTAL32\","
        "\"34\": \"GPIO34 - ADC1_6\","
        "\"35\": \"GPIO35 - ADC1_7\","
        "\"36\": \"GPIO36 - ADC1_0,SensVP\","
        "\"39\": \"GPIO39 - ADC1_3,SensVN\""
      "},"
      "\"Default\": {"
        "\"pin_rx\": 16,"
        "\"pin_tx\": 17,"
        "\"pin_therm\": 34,"
        "\"pin_sg1\": 32,"
        "\"pin_sg2\": 33,"
        "\"pin_enable_config\": 39,"
        "\"frequency\": 30000,"
        "\"mqtt_onetopic\": \"espaltherma/OneATTR/\","
        "\"mqtt_port\": 1883"        
      "}"
    "}";
#else
  const String response = "{\"Pins\": {}, \"Default\": {}}"; 
#endif

  request->send(200, "application/json", response);
}

void onIndex(AsyncWebServerRequest *request)
{    
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", indexHTML_start, indexHTML_end - indexHTML_start);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
}

void onRequestPicoCSS(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/css", picoCSS_start, picoCSS_end - picoCSS_start);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
}

void onRequestMainCSS(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/css", mainCSS_start, mainCSS_end - mainCSS_start);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
}

void onRequestMainJS(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/javascript", mainJS_start, mainJS_end - mainJS_start);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
}

void onRequestMD5JS(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/javascript", md5JS_start, md5JS_end - md5JS_start);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
}

void onFormat(AsyncWebServerRequest *request)
{
  bool result = formatDefaultFS();

  request->onDisconnect([]()
  {
#ifdef ESP32
    ESP.restart();
#elif defined(ESP8266)
    ESP.reset();
#endif
  });

  request->send(200, "text/javascript", String(result ? "OK" : "FAILED"));  
}

void onLoadModels(AsyncWebServerRequest *request)
{
  request->send(LittleFS, MODELS_FILE, "text/json");
}

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t* data, size_t len, bool final)
{
  String logmessage;  
  String fsFilename;
  
  if (!index) 
  {      
    do
    {
      fsFilename = "/P" + String(millis()) + ".json";
    } while (LittleFS.exists(fsFilename));

    logmessage = "Upload Start: " + String(filename);
    // open the file on first call and store the file handle in the request object
    request->_tempFile = LittleFS.open(fsFilename, "w");
    Serial.println(logmessage);
  }

  if (len) 
  {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);    
    logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
    Serial.println(logmessage);
  }

  if (final) 
  {       
    lastUploadFileName = "/" + String(request->_tempFile.name());

    logmessage = "Upload Complete: " + String(filename) + ", size: " + String(index + len);    
    Serial.println(logmessage);

    // close the file handle as the upload is now done
    request->_tempFile.close();    

    // minimize json file
    File modelsFile = LittleFS.open(lastUploadFileName, FILE_READ);
    DynamicJsonDocument modelsDoc(MODEL_DEFINITION_UOLOAD_SIZE);
    deserializeJson(modelsDoc, modelsFile); 
    modelsFile.close();

    modelsFile = LittleFS.open(lastUploadFileName, FILE_WRITE);
    serializeJson(modelsDoc, modelsFile);
    size_t newFileSize = modelsFile.size();
    modelsFile.close();

    logmessage = "JSON Minify Complete: " + String(lastUploadFileName) + ", new size: " + String(newFileSize);    
    Serial.println(logmessage);
  }
}

void onUpload(AsyncWebServerRequest *request)
{
  if(!request->hasParam("file", true, true))
  {
    request->send(422, "text/plain", "Missing parameter file");
    return;
  }  
  
  String fsFilename = lastUploadFileName;
  Serial.printf("Found LittleFS Filename: %s\n", fsFilename);
  
  File modelsFile = LittleFS.open(MODELS_FILE, FILE_READ);
  DynamicJsonDocument modelsDoc(MODELS_DOC_SIZE);
  deserializeJson(modelsDoc, modelsFile); 
  JsonArray modelsDocArr = modelsDoc.as<JsonArray>();
  modelsFile.close();
  
  File uploadFileFS = LittleFS.open(fsFilename, FILE_READ);
  DynamicJsonDocument uploadDoc(MODEL_DEFINITION_DOC_SIZE);
  deserializeJson(uploadDoc, uploadFileFS); 
  uploadFileFS.close();
  
  bool newModel = true;
  for (JsonObject model : modelsDocArr) 
  {
    if(strcmp(model["Model"].as<const char*>(), uploadDoc["Model"].as<const char*>()) == 0)
    {
      Serial.printf("Found existing Model: %s\n", model["Model"].as<const char*>());

      newModel = false;

      bool existingLanguage = false;
      for (JsonPair kv : model["Files"].as<JsonObject>()) 
      {
        if(strcmp(kv.key().c_str(), uploadDoc["Language"].as<const char*>()) == 0)
        {
          Serial.printf("Found existing Model file: %s\n", kv.key().c_str());
          fsFilename = kv.value().as<const char*>();
          existingLanguage = true;
          break;
        }
      }

      if(!existingLanguage)
      {
        Serial.printf("add new language to existing Model file: %s\n", uploadDoc["Language"].as<const char*>());
        model["Files"][uploadDoc["Language"].as<const char *>()] = fsFilename;
      }
    }    
  }

  if(newModel)
  {
    Serial.printf("Found new Model: %s\n", uploadDoc["Model"].as<const char*>());

    JsonObject newModelObect = modelsDocArr.createNestedObject();
    newModelObect["Model"] = uploadDoc["Model"].as<const char*>();
    newModelObect["Files"][uploadDoc["Language"].as<const char *>()] = fsFilename;
  }
  
  serializeJson(modelsDoc, Serial);

  modelsFile = LittleFS.open(MODELS_FILE, FILE_WRITE);
  serializeJson(modelsDoc, modelsFile);
  modelsFile.close();

  request->send(200);
}


void onImportConfig(AsyncWebServerRequest *request)
{
  if(!request->hasParam("file", true, true))
  {
    request->send(422, "text/plain", "Missing config file");
    return;
  }  
  
  String fsFilename = lastUploadFileName;
  Serial.printf("Found LittleFS Filename: %s\n", fsFilename);

  if(LittleFS.exists(CONFIG_FILE))    
  {
    LittleFS.remove(CONFIG_FILE);
  }

  LittleFS.rename(lastUploadFileName, CONFIG_FILE);

  request->onDisconnect([]()
  {
#ifdef ESP32
    ESP.restart();
#elif defined(ESP8266)
    ESP.reset();
#endif
  });
          
  request->send(200);
}

void onLoadValuesResult(AsyncWebServerRequest *request)
{
  if(valueLoadState == NotLoading)
  {
    request->send(503, "text/plain", "No values loading in progress");
    return;
  }

  if(valueLoadState == Loading || valueLoadState == Pending)
  {
    request->send(503, "text/plain", "Values loading not finished");
    return;
  }

  request->send(200, "application/json", valueLoadResponse);
  valueLoadResponse = "";

  valueLoadState = NotLoading;
}

void onLoadValues(AsyncWebServerRequest *request)
{
  if(!request->hasParam("PIN_RX", true) || !request->hasParam("PIN_TX", true) || !request->hasParam("PARAMS", true))
  {
    request->send(422, "text/plain", "Missing parameters PIN_RX, PIN_TX or PARAMS");
    return;
  }

  if(valueLoadState != NotLoading)
  {
    request->send(202, "text/plain", "Value loading in progress");
    return;
  }

  webuiScanRegisterConfig.PinRx = request->getParam("PIN_RX", true)->value().toInt();
  webuiScanRegisterConfig.PinTx = request->getParam("PIN_TX", true)->value().toInt();
  webuiScanRegisterConfig.Params = request->getParam("PARAMS", true)->value();  

  valueLoadState = Pending;

  request->send(200, "application/json", "OK");
}

void onLoadModel(AsyncWebServerRequest *request)
{
  if(!request->hasParam("modelFile", true))
  {
    request->send(422, "text/plain", "Missing model file");
    return;
  }
  
  String modelFile = request->getParam("modelFile", true)->value();

  Serial.print("Found model file: ");
  Serial.println(modelFile);

  if(!LittleFS.exists(modelFile))
  {
    request->send(400, "text/plain", "Model file not found");
    return;
  }

  request->send(LittleFS, modelFile, "text/json");
}

void onLoadConfig(AsyncWebServerRequest *request)
{  
  if(!LittleFS.exists(CONFIG_FILE))
  {    
    request->send(200, "text/json", "{}");
    return;
  }

  request->send(LittleFS, CONFIG_FILE, "text/json");
}

void onExportConfig(AsyncWebServerRequest *request)
{  
  AsyncWebServerResponse *response;

  if(!LittleFS.exists(CONFIG_FILE))
  {    
    response = request->beginResponse(200, "text/json", "{}");
    response->addHeader("Content-Disposition", "attachment; filename=\"config.json\"");
    response->addHeader("Content-Length", "2");
    request->send(response);
    return;
  }

  File file = LittleFS.open(CONFIG_FILE, "r");
  size_t filesize = file.size();
  file.close();

  response = request->beginResponse(LittleFS, CONFIG_FILE, "text/json", true);
  response->addHeader("Content-Disposition", "attachment; filename=\"config.json\"");
  response->addHeader("Content-Length", String(filesize));
  request->send(response);
}

void onSaveConfig(AsyncWebServerRequest *request)
{  
  #pragma region Validate_Input_Params  
  if(!request->hasParam("standalone_wifi", true))
  {
    if(!request->hasParam("ssid", true) || !request->hasParam("ssid_password", true))
    {
      request->send(422, "text/plain", "Missing parameter(s) for ssid!");
      return;
    }

    if(request->hasParam("ssid_staticip", true) && (!request->hasParam("ssid_ip", true) || 
                                                    !request->hasParam("ssid_subnet", true) || 
                                                    !request->hasParam("ssid_gateway", true) || 
                                                    !request->hasParam("primary_dns", true) || 
                                                    !request->hasParam("secondary_dns", true)))
    {
      request->send(422, "text/plain", "Missing parameter(s) for static ip");
      return;
    }
  }

  if(!request->hasParam("mqtt_server", true) || !request->hasParam("mqtt_username", true) || !request->hasParam("mqtt_password", true) || !request->hasParam("mqtt_port", true) || !request->hasParam("frequency", true))
  {
    request->send(422, "text/plain", "Missing parameter(s) for MQTT!");
    return;
  }

  if(request->hasParam("mqtt_use_onetopic", true) && !request->hasParam("mqtt_onetopic", true))
  {
    request->send(422, "text/plain", "Missing parameter(s) for MQTT onetopic");
    return;
  }

  if(!request->hasParam("pin_rx", true) || !request->hasParam("pin_tx", true) || !request->hasParam("pin_therm", true))
  {
    request->send(422, "text/plain", "Missing parameter(s) for MQTT onetopic");
    return;
  }

  if(request->hasParam("sg_enabled", true) && (!request->hasParam("pin_sg1", true) || !request->hasParam("pin_sg2", true)))
  {
    request->send(422, "text/plain", "Missing parameter(s) for SmartGrid");
    return;
  }

  if(!request->hasParam("pin_enable_config", true))
  {
    request->send(422, "text/plain", "Missing parameter pin to enable config");
    return;
  }

  if(!request->hasParam("pin_enable_config", true))
  {
    request->send(422, "text/plain", "Missing parameter pin to enable config");
    return;
  }
  #pragma endregion Validate_Input_Params

  if(config)
    delete config;
  
  config = new Config();
  config->configStored = true;
  config->STANDALONE_WIFI = request->hasParam("standalone_wifi", true);

  if(!config->STANDALONE_WIFI)
  {
    config->SSID = (char *)request->getParam("ssid", true)->value().c_str();
    config->SSID_PASSWORD = (char *)request->getParam("ssid_password", true)->value().c_str();

    config->SSID_STATIC_IP = request->hasParam("ssid_staticip", true);
    if(config->SSID_STATIC_IP)
    {
      config->SSID_IP = (char *)request->getParam("ssid_ip", true)->value().c_str();
      config->SSID_SUBNET = (char *)request->getParam("ssid_subnet", true)->value().c_str();
      config->SSID_GATEWAY = (char *)request->getParam("ssid_gateway", true)->value().c_str();
      config->SSID_PRIMARY_DNS = (char *)request->getParam("primary_dns", true)->value().c_str();
      config->SSID_SECONDARY_DNS = (char *)request->getParam("secondary_dns", true)->value().c_str();
    }
  }
  
  config->MQTT_SERVER = (char *)request->getParam("mqtt_server", true)->value().c_str();
  config->MQTT_USERNAME = (char *)request->getParam("mqtt_username", true)->value().c_str();
  config->MQTT_PASSWORD = (char *)request->getParam("mqtt_password", true)->value().c_str();
  config->FREQUENCY = request->getParam("frequency", true)->value().toInt();
  config->MQTT_USE_JSONTABLE = request->hasParam("mqtt_jsontable", true);
  config->MQTT_USE_ONETOPIC = request->hasParam("mqtt_use_onetopic", true);

  if(config->MQTT_USE_ONETOPIC)
  {
    config->MQTT_ONETOPIC_NAME = (char *)request->getParam("mqtt_onetopic", true)->value().c_str();
  }

  config->MQTT_PORT = request->getParam("mqtt_port", true)->value().toInt();
  config->PIN_RX = request->getParam("pin_rx", true)->value().toInt();
  config->PIN_TX = request->getParam("pin_tx", true)->value().toInt();
  config->PIN_THERM = request->getParam("pin_therm", true)->value().toInt();  
  config->SG_ENABLED = request->hasParam("sg_enabled", true);

  if(config->SG_ENABLED)
  {
    config->PIN_SG1 = request->getParam("pin_sg1", true)->value().toInt();
    config->PIN_SG2 = request->getParam("pin_sg2", true)->value().toInt();
  }

  config->SG_RELAY_HIGH_TRIGGER = request->hasParam("sg_relay_trigger", true);
  config->PIN_ENABLE_CONFIG = request->getParam("pin_enable_config", true)->value().toInt();
  
  if(request->hasParam("definedParameters", true))
  {
    DynamicJsonDocument jsonParameters(MODELS_CONFIG_SIZE);
    deserializeJson(jsonParameters, request->getParam("definedParameters", true)->value()); 
    JsonArray parametersArray = jsonParameters.as<JsonArray>();

    config->PARAMETERS_LENGTH = parametersArray.size();
    config->PARAMETERS = new LabelDef*[config->PARAMETERS_LENGTH];

    int counter = 0;
    for (JsonArray value : parametersArray) 
    {
      config->PARAMETERS[counter] = new LabelDef(
        value[0].as<const int>(), 
        value[1].as<const int>(), 
        value[2].as<const int>(), 
        value[3].as<const int>(), 
        value[4].as<const int>(),
        value[5]);
      counter++;
    }
  }
  else
  {
    config->PARAMETERS_LENGTH = 0;
    config->PARAMETERS = nullptr;
  }

  StaticJsonDocument<WEBUI_SELECTION_VALUE_SIZE> webuiSelectionValues;
  webuiSelectionValues["model"] = (char *)request->getParam("model", true)->value().c_str();
  webuiSelectionValues["language"] = (char *)request->getParam("language", true)->value().c_str();
  webuiSelectionValues["presetParameters"] = (char *)request->getParam("presetParameters", true)->value().c_str();

  String serializedWebuiSelectionValues;
  serializeJson(webuiSelectionValues, serializedWebuiSelectionValues);
  config->WEBUI_SELECTION_VALUES = (char *)serializedWebuiSelectionValues.c_str();  

  saveConfig();

  request->onDisconnect([]()
  {
#ifdef ESP32
    ESP.restart();
#elif defined(ESP8266)
    ESP.reset();
#endif
  });

  request->send(200, "text/plain", "OK");
}

void onUpdate(AsyncWebServerRequest *request)
{  
  request->onDisconnect([]()
  {
    if(Update.hasError())
      return;

#ifdef ESP32
    ESP.restart();
#elif defined(ESP8266)
    ESP.reset();
#endif
  });

  AsyncWebServerResponse *response = request->beginResponse((hasError)?500:200, "text/plain", (hasError)?"FAIL":"OK");
  response->addHeader("Connection", "close");
  response->addHeader("Access-Control-Allow-Origin", "*");
  request->send(response);
}

void handleUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  //Upload handler chunks in data       
  if (!index) 
  {
    Serial.print("Start Web OTA Update - MD5: ");    

    if(!request->hasParam("MD5", true)) 
    {
      return request->send(400, "text/plain", "MD5 parameter missing");
    }

    const char* md5 = request->getParam("MD5", true)->value().c_str();

    if(strlen(md5) != 32)
    {
      return request->send(400, "text/plain", "MD5 parameter invalid");
    }

    Serial.println(md5);
   
    #if defined(ESP8266)
        int cmd = (filename == "filesystem") ? U_FS : U_FLASH;
        Update.runAsync(true);
        size_t fsSize = ((size_t) &_FS_end - (size_t) &_FS_start);
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin((cmd == U_FS)?fsSize:maxSketchSpace, cmd)){ // Start with max available size
    #elif defined(ESP32)
        int cmd = (request->getParam("type", true)->value() == "filesystem") ? U_SPIFFS : U_FLASH;
        if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) { // Start with max available size
    #endif
        Update.printError(Serial);
        return request->send(400, "text/plain", "OTA could not begin");
    }
    Update.setMD5(md5);
    webOTAIsBusy = true;
  }

  // Write chunked data to the free sketch space
  if(len)
  {
    if (Update.write(data, len) != len) 
    {
        return request->send(400, "text/plain", "OTA could not begin");
    }
    Serial.print("."); 
  }
      
  if (final) 
  { // if the final flag is set then this is the last frame of data
    Serial.print("\n--> Update finished!\n");  
    webOTAIsBusy = false;
    if (!Update.end(true)) 
    { //true to set the size to the current progress
      Update.printError(Serial);
      return request->send(400, "text/plain", "Could not end OTA");
    }
  }
}

void WebUI_Init()
{
  if(!LittleFS.exists(MODELS_FILE))
  {
    formatDefaultFS();
  }

  server.on("/", HTTP_GET, onIndex);
  server.on("/pico.min.css", HTTP_GET, onRequestPicoCSS);
  server.on("/main.css", HTTP_GET, onRequestMainCSS);
  server.on("/main.js", HTTP_GET, onRequestMainJS);
  server.on("/md5.min.js", HTTP_GET, onRequestMD5JS);
  server.on("/loadModel", HTTP_POST, onLoadModel);
  server.on("/loadBoardInfo", HTTP_GET, onLoadBoardInfo);
  server.on("/upload", HTTP_POST, onUpload, handleUpload);
  server.on("/loadModels", HTTP_GET, onLoadModels);
  server.on("/loadValues", HTTP_POST, onLoadValues);
  server.on("/loadValuesResult", HTTP_GET, onLoadValuesResult);
  server.on("/saveConfig", HTTP_POST, onSaveConfig);
  server.on("/exportConfig", HTTP_GET, onExportConfig);
  server.on("/importConfig", HTTP_POST, onImportConfig, handleUpload);
  server.on("/loadConfig", HTTP_GET, onLoadConfig);
  server.on("/loadWifiNetworks", HTTP_GET, onLoadWifiNetworks);
  server.on("/format", HTTP_GET, onFormat);
  server.on("/update", HTTP_POST, onUpdate, handleUpdate);  
  server.begin();
}
#endif