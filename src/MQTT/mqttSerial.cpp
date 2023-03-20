#include "mqttSerial.hpp"

MQTTSerial mqttSerial;

MQTTSerial::MQTTSerial()
{
}

size_t MQTTSerial::write(const uint8_t *buffer, size_t size)
{
#ifdef ARDUINO_M5Stick_C
    if (M5.Lcd.getCursorY()+13>M5.Lcd.height()){
        M5.Lcd.fillScreen(TFT_BLACK);
        M5.Lcd.setCursor(0,0);
    }
    M5.Lcd.print((const char*) buffer);
#endif

    if (WiFi.status() == WL_CONNECTED && _client!=nullptr &&_client->connected())
    {
        _client->publish(_topic,buffer,size);
    }

    WebSerial.write(buffer,size);
    Serial.write(buffer,size);

    return size;
}

MQTTSerial::~MQTTSerial()
{
}