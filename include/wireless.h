#ifndef WIRELESS_H
#define WIRELESS_H
#include <WiFi.h>
#include <string.h>
#include "mqttSerial.h"
#include "config.h"

struct WifiDetails
{
  const String SSID;    
  const int32_t RSSI;
  const wifi_auth_mode_t EncryptionType;
};

void start_standalone_wifi()
{  
  IPAddress local_ip(192, 168, 1, 1); 
  IPAddress gateway(192, 168, 1, 1); 
  IPAddress subnet(255, 255, 255, 0);
  WiFi.softAP("ESPAltherma-Config-WiFi");   
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.setHostname("ESPAltherma");    
}

void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  mqttSerial.printf("Connecting to %s\n", config->SSID);

  if(config->SSID_STATIC_IP)
  {
    IPAddress local_IP;
    IPAddress subnet;
    IPAddress gateway;
    IPAddress primaryDNS;
    IPAddress secondaryDNS;

    local_IP.fromString(config->SSID_IP);
    subnet.fromString(config->SSID_SUBNET);
    gateway.fromString(config->SSID_GATEWAY);

    if(config->SSID_PRIMARY_DNS != "")
    {
      primaryDNS.fromString(config->SSID_PRIMARY_DNS);
    }

    if(config->SSID_SECONDARY_DNS != "")
    {
      secondaryDNS.fromString(config->SSID_SECONDARY_DNS);
    }

    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
        mqttSerial.println("Failed to set static ip!");
    }
  }

  WiFi.begin(config->SSID.c_str(), config->SSID_PASSWORD.c_str());
  int i = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    if (i++ == 100)
    {
      esp_restart();
    }
  }
  mqttSerial.printf("Connected. IP Address: %s\n", WiFi.localIP().toString().c_str());
}

WifiDetails **lastWifiScanResults = nullptr;
int16_t lastWifiScanResultAmount;

void scan_wifi_delete_result()
{
  for (int16_t i = 0; i < lastWifiScanResultAmount; i++) {
    delete lastWifiScanResults[i];
  }

  delete[] lastWifiScanResults;
}

void scan_wifi()
{  
  lastWifiScanResultAmount = WiFi.scanNetworks();
  lastWifiScanResults = new WifiDetails*[lastWifiScanResultAmount];

  for (int16_t i = 0; i < lastWifiScanResultAmount; i++) {
    lastWifiScanResults[i] = new WifiDetails {
      .SSID = WiFi.SSID(i),
      .RSSI = WiFi.RSSI(i),
      .EncryptionType = WiFi.encryptionType(i)
    };

    mqttSerial.print(lastWifiScanResults[i]->SSID);
    String serialLog = " (" + String(lastWifiScanResults[i]->RSSI) + ") " + lastWifiScanResults[i]->EncryptionType + "\n";
    mqttSerial.print(serialLog);
    delay(10);
  }
}

#endif