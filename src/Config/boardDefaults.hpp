#ifndef BOARD_DEFAULTS_H
#define BOARD_DEFAULTS_H

const char* const JSON_BOARD_DEFAULTS =

#if defined(ARDUINO_M5Stick_C)
    "\"Default\": {"
      "\"pin_rx\": 36,"
      "\"pin_tx\": 26,"
      "\"pin_heating\": 0,"
      "\"pin_cooling\": 0,"
      "\"pin_sg1\": 32,"
      "\"pin_sg2\": 33,"
      "\"uart\": {"
        "\"pin_rx\": 32,"
        "\"pin_tx\": 33"
      "},"
      "\"spi\": {"
        "\"mosi\": 26,"
        "\"miso\": 0,"
        "\"sck\": 36,"
        "\"cs\": 32,"
        "\"int\": 33,"
        "\"mhz\": 12"
      "},"
      "\"can_speed_kbps\": 20,"
      "\"can_mqtt_topic_name\": \"CAN/\","
      "\"can_autopoll_time\": 30,"
      "\"pin_enable_config\": 39,"
      "\"frequency\": 30000,"
      "\"mqtt_topic_name\": \"espaltherma/\","
      "\"mqtt_onetopic_name\": \"OneATTR/\","
      "\"mqtt_port\": 1883"
    "}"
#elif defined(ESP32)
    "\"Default\": {"
      "\"pin_rx\": 16,"
      "\"pin_tx\": 17,"
      "\"pin_heating\": 13,"
      "\"pin_cooling\": 14,"
      "\"pin_sg1\": 32,"
      "\"pin_sg2\": 33,"
      "\"uart\": {"
        "\"pin_rx\": 4,"
        "\"pin_tx\": 5"
      "},"
      "\"spi\": {"
        "\"mosi\": 23,"
        "\"miso\": 19,"
        "\"sck\": 18,"
        "\"cs\": 5,"
        "\"int\": 4,"
        "\"mhz\": 12"
      "},"
      "\"can_speed_kbps\": 20,"
      "\"can_mqtt_topic_name\": \"CAN/\","
      "\"can_autopoll_time\": 30,"
      "\"pin_enable_config\": 27,"
      "\"frequency\": 30000,"
      "\"mqtt_topic_name\": \"espaltherma/\","
      "\"mqtt_onetopic_name\": \"OneATTR/\","
      "\"mqtt_port\": 1883"
    "}"
#else
    "\"Default\": {}"
#endif
;

#endif