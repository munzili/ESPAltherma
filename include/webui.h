#include <WiFi.h>
#include "ESPAsyncWebServer.h" 
#include "SPIFFS.h"

// Set web server port number to 80
AsyncWebServer server(80);

const String WEBUI_ROOTDIR = "/include/webui/";

String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    return "ON";
  }
  return String();
}

void onIndex(AsyncWebServerRequest *request)
{
    request->send(SPIFFS, WEBUI_ROOTDIR + "index.html", String(), false, processor);
}

void onStyle(AsyncWebServerRequest *request)
{
    request->send(SPIFFS, WEBUI_ROOTDIR + "style.css", "text/css");
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
        Serial.println("An Error has occurred while mounting SPIFFS");
    }
    
    server.on("/", HTTP_GET, onIndex);
    server.on("/style.css", HTTP_GET, onStyle);
    server.on("/save", HTTP_POST, onSave);
    server.begin();     
}