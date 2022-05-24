#include <WiFi.h>
#include <SPIFFS.h>
#include "ESPAsyncWebServer.h" 
#include "ArduinoJson.h"

// Set web server port number to 80
AsyncWebServer server(80);

String processor(const String& var) {
  Serial.println(var);
  if(var == "STATE"){
    return "ON";
  }
  if(var == "MODELS")
  {
    File file = SPIFFS.open("/def/models.json");

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
    File file = SPIFFS.open("/def/languages.json");

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
    request->send(SPIFFS, "/index.html", "text/html", false, processor);
}

void onRequestPicoCSS(AsyncWebServerRequest *request)
{
    request->send(SPIFFS, "/pico.min.css", "text/css");
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
    byte language = parameterLanguage->value().toInt();
    Serial.print("Found model: ");
    Serial.println(model);
    Serial.print("Found language: ");
    Serial.println(language);

    File file = SPIFFS.open("/def/models.json");

    StaticJsonDocument<1024*10> modelDoc;
    deserializeJson(modelDoc, file); 
    JsonObject obj = modelDoc.as<JsonArray>()[model];
    const char* modelFile = obj["File"].as<const char*>();

    file.close();

    String filename = "/def/";  

    if(language != 1)
    {
      file = SPIFFS.open("/def/languages.json");

      StaticJsonDocument<512> languagesDoc;
      deserializeJson(languagesDoc, file); 
      const char* languageName = languagesDoc.as<JsonArray>()[language-2].as<const char*>();

      file.close();

      filename += languageName;
      filename += "/";
    }

    filename += modelFile;
    filename += ".json";

    Serial.println("Searching: " + filename);
    
    if(!SPIFFS.exists(filename))
    {
      filename = "/def/";  
      filename += modelFile;
      filename += ".json";
    }

    if(!SPIFFS.exists(filename))
    {
      request->send(400, "text/text", "Parameters file not found");
      return;
    }

    request->send(SPIFFS, filename, "text/json");
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
    if(!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting LittleFS");
    }
    
    server.on("/", HTTP_GET, onIndex);
    server.on("/pico.min.css", HTTP_GET, onRequestPicoCSS);
    server.on("/loadParameters", HTTP_POST, onLoadParameters);
    server.on("/save", HTTP_POST, onSave);
    server.begin();     
}