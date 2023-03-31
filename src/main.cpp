#include "main.hpp"

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

  esp_chip_info_t chip;
  esp_chip_info(&chip);

  mqttSerial.printf("ESP32 Model: %i\n", chip.model);
  mqttSerial.printf("ESP32 Revision: %i\n", chip.revision);
  mqttSerial.printf("ESP32 Cores: %i\n", chip.cores);

  initPersistence();

  readConfig();

  if(config->STANDALONE_WIFI || !config->configStored)
  {
    mqttSerial.println("Start in standalone mode..");
    start_standalone_wifi();
    WebUI_Init();
  }

  initMQTT();

  setupScreen();

  if(!config->configStored)
  {
    mqttSerial.println("No config found, skip setup...");
    return;
  }

  if(config->X10A_ENABLED)
  {
    X10AInit(config->PIN_RX, config->PIN_TX);
    initRegistries(&registryBuffers, registryBufferSize, config->PARAMETERS, config->PARAMETERS_LENGTH);
  }

  if(config->HEATING_ENABLED)
  {
    pinMode(config->PIN_HEATING, OUTPUT);
    digitalWrite(config->PIN_HEATING, HIGH);
  }

  if(config->COOLING_ENABLED)
  {
    pinMode(config->PIN_COOLING, OUTPUT);
    digitalWrite(config->PIN_COOLING, HIGH);
  }

  if(config->SG_ENABLED)
  {
    //Smartgrid pins - Set first to the inactive state, before configuring as outputs (avoid false triggering when initializing)
    digitalWrite(config->PIN_SG1, SG_RELAY_INACTIVE_STATE);
    digitalWrite(config->PIN_SG2, SG_RELAY_INACTIVE_STATE);
    pinMode(config->PIN_SG1, OUTPUT);
    pinMode(config->PIN_SG2, OUTPUT);

    mqttSerial.printf("Configured SG Pins %u %u - State: %u\n", config->PIN_SG1, config->PIN_SG2, SG_RELAY_INACTIVE_STATE);
  }

  if(config->CAN_ENABLED)
  {
    canBus_setup();
  }

#ifdef ARDUINO_M5Stick_C_Plus
  gpio_pulldown_dis(GPIO_NUM_25);
  gpio_pullup_dis(GPIO_NUM_25);
#endif

  readPersistence();//Restore previous state

  if(!config->STANDALONE_WIFI)
  {
    mqttSerial.println("Setting up wifi...");
    setup_wifi();
    WebUI_Init();
  }

  pinMode(config->PIN_ENABLE_CONFIG, INPUT_PULLUP);
  attachInterrupt(config->PIN_ENABLE_CONFIG, restartInStandaloneWifi, FALLING);

  mqttSerial.print("Connecting to MQTT server...\n");
  reconnect();
  mqttSerial.println("OK!");

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

  if(config->X10A_ENABLED)
  {
    handleX10A(registryBuffers, registryBufferSize, config->PARAMETERS, config->PARAMETERS_LENGTH, true);
  }

  mqttSerial.printf("Done. Waiting %d sec...\n", config->FREQUENCY / 1000);
  waitLoop(config->FREQUENCY);
}