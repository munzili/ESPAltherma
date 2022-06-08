#ifndef webui_h
#define webui_h
#include <WiFi.h>
#include <LittleFS.h>
#include "ESPAsyncWebServer.h" 
#include "ArduinoJson.h"
#include "comm.h"
#include "esp_task_wdt.h"

#define MODELS_FILE "/models.json"
#define MODEL_DEFINITION_DOC_SIZE 1024*50
#define MODELS_DOC_SIZE 1024*10

// Set web server port number to 80
AsyncWebServer server(80);

extern const char mainJS_start[] asm("_binary_webui_main_js_start");
extern const char indexHTML_start[] asm("_binary_webui_index_html_start");
extern const char picoCSS_start[] asm("_binary_webui_pico_min_css_start");

String lastUploadFileName;

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

  return true;
}

void onIndex(AsyncWebServerRequest *request)
{
    request->send_P(200, "text/html", indexHTML_start);
}

void onRequestPicoCSS(AsyncWebServerRequest *request)
{
    request->send_P(200, "text/css", picoCSS_start);
}

void onRequestMainJS(AsyncWebServerRequest *request)
{
    request->send_P(200, "text/javascript", mainJS_start);
}

void onFormat(AsyncWebServerRequest *request)
{
  bool result = formatDefaultFS();

  request->send(200, "text/javascript", String(result));

  esp_restart();
}

void onLoadModels(AsyncWebServerRequest *request)
{
  request->send(LittleFS, MODELS_FILE, "text/json");
}

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t* data, size_t len, bool final)
{
  String logmessage;  
  String fsFilename;
  
  if (!index) {
      
    do
    {
      fsFilename = "/P" + String(millis()) + ".json";
    } while (LittleFS.exists(fsFilename));

    logmessage = "Upload Start: " + String(filename);
    // open the file on first call and store the file handle in the request object
    request->_tempFile = LittleFS.open(fsFilename, "w");
    Serial.println(logmessage);
  }

  if (len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);    
    logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
    Serial.println(logmessage);
  }

  if (final) {       
    lastUploadFileName = "/" + String(request->_tempFile.name());

    logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
    // close the file handle as the upload is now done
    request->_tempFile.close();
    Serial.println(logmessage);
  }
}

void onUpload(AsyncWebServerRequest *request)
{
  if(!request->hasParam("file", true, true))
  {
    request->send(422, "text/text", "Missing parameter file");
    return;
  }  
  
  String fsFilename = lastUploadFileName;
  Serial.printf("Found LitteFS Filename: %s\n", fsFilename);
  
  File modelsFile = LittleFS.open(MODELS_FILE, FILE_READ);
  DynamicJsonDocument modelsDoc(MODELS_DOC_SIZE);
  deserializeJson(modelsDoc, modelsFile.readString()); 
  JsonArray modelsDocArr = modelsDoc.as<JsonArray>();
  modelsFile.close();
  
  File uploadFileFS = LittleFS.open(fsFilename, FILE_READ);
  DynamicJsonDocument uploadDoc(MODEL_DEFINITION_DOC_SIZE);
  deserializeJson(uploadDoc, uploadFileFS.readString()); 
  uploadFileFS.close();
  
  bool newModel = true;
  for (JsonObject model : modelsDocArr) {
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

void onLoadValues(AsyncWebServerRequest *request)
{
  if(!request->hasParam("PIN_RX", true) || !request->hasParam("PIN_TX", true) || !request->hasParam("PARAMS", true))
  {
    request->send(422, "text/text", "Missing parameters PIN_RX, PIN_TX or PARAMS");
    return;
  }

  esp_task_wdt_delete(NULL);
  
  if(SerialX10A)
  {
    Serial.println("Canceling current serial connction");
    SerialX10A.end();
  }

  int8_t pinRx = request->getParam("PIN_RX", true)->value().toInt();
  int8_t pinTx = request->getParam("PIN_TX", true)->value().toInt();
  String params = request->getParam("PARAMS", true)->value();

  Serial.printf("Starting new serial connection with pins RX: %u, TX: %u\n", pinRx, pinTx);

  SerialX10A.begin(
    9600, 
    SERIAL_8E1, 
    pinRx, 
    pinTx);

  DynamicJsonDocument modelsDoc(MODELS_DOC_SIZE);
  deserializeJson(modelsDoc, params); 
  JsonArray modelsDocArr = modelsDoc.as<JsonArray>();

  Serial.printf("Creating labelDefs %i\n", modelsDocArr.size());

  // temp create new label definitions from request and afterwards restore old one
  LabelDef ***oldLabelDefs = &labelDefs;
  uint8_t oldLabelDefsSize = labelDefsSize;
  labelDefsSize = modelsDocArr.size();
  labelDefs = new LabelDef*[labelDefsSize];
  
  uint8_t counter = 0;
  for (JsonArray model : modelsDocArr) {
    labelDefs[counter] = new LabelDef(model[0], model[1], model[2], model[3], model[4], model[5]);
    counter++;
  }  

  Serial.println("Fetching Values");

  for (JsonArray model : modelsDocArr) {
    char buff[64] = {0};
    int tries = 0;
    Serial.printf("Quering register %i\n", model[0].as<const uint8_t>());
    while (tries++ < 3 && !queryRegistry(model[0].as<const uint8_t>(), buff))
    {
      mqttSerial.println("Retrying...");
      delay(1000);
    }
    if (model[0].as<const uint8_t>() == buff[1]) //if replied registerID is coherent with the command
    {
      mqttSerial.println("Found value " + buff[1]);
      converter.readRegistryValues(buff); //process all values from the register
    }
  }  

  Serial.println("Returning Values");

  DynamicJsonDocument resultDoc(modelsDocArr.size()*JSON_OBJECT_SIZE(2));
  JsonArray obj = resultDoc.to<JsonArray>();
  
  for (uint8_t i = 0; i < counter; i++) {
    obj.add(labelDefs[i]->asString);
  } 

  Serial.println("Delete tmp labelDefs");

  while(counter >= 0)
  {
    delete[] labelDefs[counter];
    counter--;
  }

  delete[] labelDefs;

  Serial.println("Setting old labelDefs");

  labelDefs = *oldLabelDefs;
  labelDefsSize = oldLabelDefsSize;

  String response;
  serializeJson(resultDoc, response);

  request->send(200, "application/json", response);
}

void onLoadParameters(AsyncWebServerRequest *request)
{
    if(!request->hasParam("parametersFile", true))
    {
      request->send(422, "text/text", "Missing parameter file");
      return;
    }
    
    String parametersFile = request->getParam("parametersFile", true)->value();

    Serial.print("Found parameter file: ");
    Serial.println(parametersFile);

    if(!LittleFS.exists(parametersFile))
    {
      request->send(400, "text/text", "Parameters file not found");
      return;
    }

    request->send(LittleFS, parametersFile, "text/json");
}

void onSave(AsyncWebServerRequest *request)
{
}

void WebUI_Init()
{
  if(!LittleFS.exists(MODELS_FILE))
  {
    formatDefaultFS();
  }
  
  server.on("/", HTTP_GET, onIndex);
  server.on("/pico.min.css", HTTP_GET, onRequestPicoCSS);
  server.on("/main.js", HTTP_GET, onRequestMainJS);
  server.on("/loadParameters", HTTP_POST, onLoadParameters);
  server.on("/upload", HTTP_POST, onUpload, handleUpload);
  server.on("/loadModels", HTTP_GET, onLoadModels);
  server.on("/loadValues", HTTP_POST, onLoadValues);
  server.on("/save", HTTP_POST, onSave);
  server.on("/format", HTTP_GET, onFormat);
  server.begin();     
}
#endif