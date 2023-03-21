#include "main.hpp"

size_t registryBufferSize;
RegistryBuffer *registryBuffers; //Holds the registries to query and the last returned data

bool doRestartInStandaloneWifi = false;

uint16_t loopcount = 0;

void extraLoop()
{
  while(webOTAIsBusy) {}

  client.loop();

  if(config->CAN_ENABLED)
    canBus_loop();

#if defined(ARDUINO_M5Stick_C) || defined(ARDUINO_M5Stick_C_Plus)
  if (M5.BtnA.wasPressed()){//Turn back ON screen
    M5.Axp.ScreenBreath(12);
    LCDTimeout = millis() + 30000;
  }else if (LCDTimeout < millis()){//Turn screen off.
    M5.Axp.ScreenBreath(0);
  }
  M5.update();
#endif

  if(!doRestartInStandaloneWifi)
   return;

  mqttSerial.println("Restarting in standalone wifi mode");
  config->STANDALONE_WIFI = true;
  saveConfig();
  esp_restart();
}

void initRegistries()
{
  //getting the list of registries to query from the selected values
  registryBufferSize = 0;
  uint8_t* tempRegistryIDs = new uint8_t[config->PARAMETERS_LENGTH];

  size_t i;
  for (i = 0; i < config->PARAMETERS_LENGTH; i++)
  {
    auto &&label = *config->PARAMETERS[i];

    if (!contains(tempRegistryIDs, config->PARAMETERS_LENGTH, label.registryID))
    {
      mqttSerial.printf("Adding registry 0x%2x to be queried.\n", label.registryID);
      tempRegistryIDs[registryBufferSize++] = label.registryID;
    }
  }

  registryBuffers = new RegistryBuffer[registryBufferSize];

  for(i = 0; i < registryBufferSize; i++)
  {
    registryBuffers[i].RegistryID = tempRegistryIDs[i];
  }

  delete[] tempRegistryIDs;
}

void setupScreen(){
#if defined(ARDUINO_M5Stick_C) || defined(ARDUINO_M5Stick_C_Plus)
  M5.begin();
  M5.Axp.EnableCoulombcounter();
  M5.Lcd.setRotation(1);
  M5.Axp.ScreenBreath(12);
  M5.Lcd.fillScreen(TFT_WHITE);
  M5.Lcd.setFreeFont(&FreeSansBold12pt7b);
  m5.Lcd.setTextDatum(MC_DATUM);
  int xpos = M5.Lcd.width() / 2; // Half the screen width
  int ypos = M5.Lcd.height() / 2; // Half the screen width
  M5.Lcd.setTextColor(TFT_DARKGREY);
  M5.Lcd.drawString("ESPAltherma", xpos, ypos, 1);
  delay(2000);
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextFont(1);
  M5.Lcd.setTextColor(TFT_GREEN);
#endif
}

void IRAM_ATTR restartInStandaloneWifi() {
  doRestartInStandaloneWifi = true;
}

void setup()
{
  Serial.begin(115200);

  if(!LittleFS.begin(true))
  {
      Serial.println("An Error has occurred while mounting LittleFS");
      return;
  }

  initPersistence();

  readConfig();

  if(config->STANDALONE_WIFI || !config->configStored)
  {
    mqttSerial.println("Start in standalone mode..");
    start_standalone_wifi();
    WebUI_Init();
  }

  initMQTTConfig(config);

  setupScreen();

  if(!config->configStored)
  {
    mqttSerial.println("No config found, skip setup...");
    return;
  }

  X10AInit(config->PIN_RX, config->PIN_TX);
  pinMode(config->PIN_HEATING, OUTPUT);
  digitalWrite(config->PIN_HEATING, HIGH);
  pinMode(config->PIN_COOLING, OUTPUT);
  digitalWrite(config->PIN_COOLING, HIGH);

  if(config->SG_ENABLED)
  {
    //Smartgrid pins - Set first to the inactive state, before configuring as outputs (avoid false triggering when initializing)
    digitalWrite(config->PIN_SG1, SG_RELAY_INACTIVE_STATE);
    digitalWrite(config->PIN_SG2, SG_RELAY_INACTIVE_STATE);
    pinMode(config->PIN_SG1, OUTPUT);
    pinMode(config->PIN_SG2, OUTPUT);
  }

  if(config->CAN_ENABLED)
  {
    canBus_setup(config->PIN_CAN_RX, config->PIN_CAN_TX, config->CAN_SPEED_KBPS);
  }

#ifdef ARDUINO_M5Stick_C_Plus
  gpio_pulldown_dis(GPIO_NUM_25);
  gpio_pullup_dis(GPIO_NUM_25);
#endif

  readPersistence();//Restore previous state

  if(!config->STANDALONE_WIFI)
  {
    mqttSerial.print("Setting up wifi...");
    setup_wifi();
    WebUI_Init();
  }

  pinMode(config->PIN_ENABLE_CONFIG, INPUT_PULLUP);
  attachInterrupt(config->PIN_ENABLE_CONFIG, restartInStandaloneWifi, FALLING);

  initMQTT();

  mqttSerial.print("Connecting to MQTT server...\n");
  mqttSerial.begin(&client, (config->MQTT_TOPIC_NAME + "log").c_str());
  reconnect();
  mqttSerial.println("OK!");

  initRegistries();
  mqttSerial.print("ESPAltherma started!\n");
}

void waitLoop(uint ms){
  unsigned long start = millis();
  while (millis() < start + ms) //wait .5sec between registries
  {
    if(valueLoadState == Pending)
      return;

    extraLoop();
  }
}

void loop()
{
  webuiScanRegister();

  if(!config->configStored)
  {
    extraLoop();
    return;
  }

  if (!client.connected())
  { //(re)connect to MQTT if needed
    reconnect();
  }

  //Querying all registries and store results
  for (size_t i = 0; i < registryBufferSize; i++)
  {
    uint8_t tries = 0;
    while (tries++ < 3 && !queryRegistry(&registryBuffers[i]))
    {
      mqttSerial.println("Retrying...");
      waitLoop(1000);
    }
  }

  for (size_t i = 0; i < config->PARAMETERS_LENGTH; i++)
  {
    auto &&label = *config->PARAMETERS[i];

    for (size_t j = 0; j < registryBufferSize; j++)
    {
      if(registryBuffers[j].Success && label.registryID == registryBuffers[j].RegistryID)
      {
        char *input = registryBuffers[j].Buffer;
        input += label.offset + 3;

        converter.convert(&label, input); // convert buffer result of label offset to correct/usabel value

        updateValues(&label);       //send them in mqtt
        waitLoop(500);//wait .5sec between registries
        break;
      }
    }
  }

  sendValues();//Send the full json message
  mqttSerial.printf("Done. Waiting %d sec...\n", config->FREQUENCY / 1000);
  waitLoop(config->FREQUENCY);
}