#include <WiFi.h>
#include <LittleFS.h>
#include "ESPAsyncWebServer.h" 
#include "ArduinoJson.h"

#define CONFIG_FILE "/config.json"
#define MODELS_FILE "/models.json"
#define MODEL_DEFINITION_DOC_SIZE 1024*50
#define MODELS_DOC_SIZE 1024*10

// Set web server port number to 80
AsyncWebServer server(80);

extern const char mainJS_start[] asm("_binary_webui_main_js_start");
extern const char indexHTML_start[] asm("_binary_webui_index_html_start");
extern const char picoCSS_start[] asm("_binary_webui_pico_min_css_start");

String processor(const String& var) {  
  if(var == "MODELS")
  {
    File file = LittleFS.open("/def/models.json");

    StaticJsonDocument<1024*10> modelDoc;
    deserializeJson(modelDoc, file); 

    String result = "";
    uint8_t counter = 1;

    // Loop through all the elements of the array
    for (JsonObject repo : modelDoc.as<JsonArray>()) {
      // Print the name, the number of stars, and the number of issues
      result += "<option value='";
      result += counter;
      result += "'>";
      result += repo["Name"].as<const char*>();
      result += "</option>";
      counter++;
    }

    file.close();

    return result;
  }
  if(var == "LANGUAGES")
  {
    File file = LittleFS.open("/def/languages.json");

    StaticJsonDocument<512> languagesDoc;
    deserializeJson(languagesDoc, file); 

    String result = "<option value='1'>GB/US</option>";
    
    uint8_t counter = 2;
    // Loop through all the elements of the array
    for (const char* arr : languagesDoc.as<JsonArray>()) {
      // Print the name, the number of stars, and the number of issues
      result += "<option value='";
      result += counter;
      result += "'>";
      result += arr;
      result += "</option>";
      counter++;
    }

    file.close();

    return result;
  }
  return String();
}

void onIndex(AsyncWebServerRequest *request)
{
    request->send_P(200, "text/html", indexHTML_start, processor);
}

void onRequestPicoCSS(AsyncWebServerRequest *request)
{
    request->send_P(200, "text/css", picoCSS_start);
}

void onRequestMainJS(AsyncWebServerRequest *request)
{
    request->send_P(200, "text/javascript", mainJS_start);
}

void onLoadModels(AsyncWebServerRequest *request)
{
  request->send(LittleFS, MODELS_FILE, "text/json");
}

void onUpload(AsyncWebServerRequest *request)
{
  if(!request->hasParam("file", true, true))
  {
    request->send(422, "text/text", "Missing parameter file");
      return;
  }

  AsyncWebParameter* uploadFile = request->getParam("file", true, true);

  File modelsFile = LittleFS.open(MODELS_FILE, "w");

  String fsFilename;
  do
  {
    fsFilename = "/P" + millis();
    fsFilename += ".json";
  } while (LittleFS.exists(fsFilename));

  DynamicJsonDocument modelsDoc(MODELS_DOC_SIZE);
  deserializeJson(modelsDoc, modelsFile); 
  JsonArray modelsDocArr = modelsDoc.as<JsonArray>();

  DynamicJsonDocument uploadDoc(MODEL_DEFINITION_DOC_SIZE);
  deserializeJson(uploadDoc, uploadFile->value()); 
  
  bool newModel = true;
  for (JsonObject model : modelsDocArr) {
    if(model["Model"] == uploadDoc["Model"])
    {
      newModel = false;

      bool existingLanguage = false;
      for (JsonPair kv : model["Files"].as<JsonObject>()) {
        if(kv.key().c_str() == uploadDoc["Language"])
        {
          fsFilename = kv.value().as<char*>();
          existingLanguage = true;
          break;
        }
      }

      if(!existingLanguage)
      {
        model["Files"][uploadDoc["Language"]] = fsFilename;
      }
    }    
  }

  if(newModel)
  {
    JsonObject newModelObect = modelsDocArr.createNestedObject();
    newModelObect["Model"] = uploadDoc["Model"];
    JsonObject filesDefinition = newModelObect["Files"].createNestedObject();
    filesDefinition[uploadDoc["Language"].as<char *>()] = fsFilename;
  }

  serializeJson(modelsDocArr, modelsFile);
  modelsFile.close();

  File fileWriter = LittleFS.open(fsFilename, "w", true);
  fileWriter.print(uploadFile->value());
  fileWriter.close();
}

void onLoadParameters(AsyncWebServerRequest *request)
{
    if(!request->hasParam("model", true) || !request->hasParam("language", true))
    {
      request->send(422, "text/text", "Missing parameter model or language");
      return;
    }

    AsyncWebParameter* parameterModel = request->getParam("model", true);
    AsyncWebParameter* parameterLanguage = request->getParam("language", true);
    
    byte model = parameterModel->value().toInt();
    String language = parameterLanguage->value();

    Serial.print("Found model: ");
    Serial.println(model);
    Serial.print("Found language: ");
    Serial.println(language);

    File file = LittleFS.open(MODELS_FILE);

    DynamicJsonDocument modelDoc(MODELS_DOC_SIZE);
    deserializeJson(modelDoc, file); 

    String parametersfile = modelDoc[model]["Files"][language];
    
    Serial.print("Found parameters: ");
    Serial.println(parametersfile);

    file.close();

    String filename = "/" + parametersfile;

    if(!LittleFS.exists(filename))
    {
      request->send(400, "text/text", "Parameters file not found");
      return;
    }

    request->send(LittleFS, filename, "text/json");
}

void onSave(AsyncWebServerRequest *request)
{
/*
int params = request->params();
for(int i=0;i<params;i++){
  AsyncWebParameter* p = request->getParam(i);
  if(p->isFile()){ //p->isPost() is also true
    Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
  } else if(p->isPost()){
    Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
  } else {
    Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
  }
}


//Check if GET parameter exists
if(request->hasParam("download"))
  AsyncWebParameter* p = request->getParam("download");

//Check if POST (but not File) parameter exists
if(request->hasParam("download", true))
  AsyncWebParameter* p = request->getParam("download", true);

//Check if FILE was uploaded
if(request->hasParam("download", true, true))
  AsyncWebParameter* p = request->getParam("download", true, true);

//List all parameters (Compatibility)
int args = request->args();
for(int i=0;i<args;i++){
  Serial.printf("ARG[%s]: %s\n", request->argName(i).c_str(), request->arg(i).c_str());
}

//Check if parameter exists (Compatibility)
if(request->hasArg("download"))
  String arg = request->arg("download");
*/
}

void WebUI_Init()
{
    if(!LittleFS.begin(true)) 
    {
        Serial.println("An Error has occurred while mounting LittleFS");
        return;
    }

    if(!LittleFS.exists(MODELS_FILE))
    {
      File file = LittleFS.open(MODELS_FILE, "w", true);
      file.print("[]");
      delay(1);
      file.close();
    }
    
    server.on("/", HTTP_GET, onIndex);
    server.on("/pico.min.css", HTTP_GET, onRequestPicoCSS);
    server.on("/main.js", HTTP_GET, onRequestMainJS);
    server.on("/loadParameters", HTTP_POST, onLoadParameters);
    server.on("/upload", HTTP_POST, onUpload);
    server.on("/loadModels", HTTP_GET, onLoadModels);
    server.on("/save", HTTP_POST, onSave);
    server.begin();     
}