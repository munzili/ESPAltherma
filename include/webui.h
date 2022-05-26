#include <WiFi.h>
#include <LittleFS.h>
#include "ESPAsyncWebServer.h" 
#include "ArduinoJson.h"

#define CONFIG_FILE "/config.json"
#define MODELS_FILE "/models.json"

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

    File file = LittleFS.open("/def/models.json");

    StaticJsonDocument<1024*10> modelDoc;
    deserializeJson(modelDoc, file); 
    JsonObject obj = modelDoc.as<JsonArray>()[model];
    const char* modelFile = obj["File"].as<const char*>();

    file.close();

    String filename = "/def/";  

    if(language != 1)
    {
      file = LittleFS.open("/def/languages.json");

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
    
    if(!LittleFS.exists(filename))
    {
      filename = "/def/";  
      filename += modelFile;
      filename += ".json";
    }

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
    server.on("/save", HTTP_POST, onSave);
    server.begin();     
}