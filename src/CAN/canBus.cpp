#include "canBus.hpp"

bool canInited = false;

static int         const MKRCAN_MCP2515_CS_PIN  = SS;
static int         const MKRCAN_MCP2515_INT_PIN = 4;
static SPISettings const MCP2515x_SPI_SETTING{1000000, MSBFIRST, SPI_MODE0};

void onReceiveBufferFull(uint32_t const, uint32_t const, uint8_t const *, uint8_t const);

typedef struct
{
  uint32_t id;
  uint8_t  data[8];
  uint8_t  len;
} sCanTestFrame;

ArduinoMCP2515 mcp2515([]()
                       {
                         SPI.beginTransaction(MCP2515x_SPI_SETTING);
                         digitalWrite(MKRCAN_MCP2515_CS_PIN, LOW);
                       },
                       []()
                       {
                         digitalWrite(MKRCAN_MCP2515_CS_PIN, HIGH);
                         SPI.endTransaction();
                       },
                       [](uint8_t const d) { return SPI.transfer(d); },
                       micros,
                       onReceiveBufferFull,
                       nullptr);

static sCanTestFrame const test_frame_1 = { 0x00000001, {0}, 0 };                                              /* Minimum (no) payload */
static sCanTestFrame const test_frame_2 = { 0x00000002, {0xCA, 0xFE, 0xCA, 0xFE, 0, 0, 0, 0}, 4 };             /* Between minimum and maximum payload */
static sCanTestFrame const test_frame_3 = { 0x00000003, {0xCA, 0xFE, 0xCA, 0xFE, 0xCA, 0xFE, 0xCA, 0xFE}, 8 }; /* Maximum payload */
static sCanTestFrame const test_frame_4 = { 0x40000004, {0}, 0 };                                              /* RTR frame */
static sCanTestFrame const test_frame_5 = { 0x000007FF, {0}, 0 };                                              /* Highest standard 11 bit CAN address */
static sCanTestFrame const test_frame_6 = { 0x80000000, {0}, 0 };                                              /* Lowest extended 29 bit CAN address */
static sCanTestFrame const test_frame_7 = { 0x9FFFFFFF, {0}, 0 };                                              /* Highest extended 29 bit CAN address */

static std::array<sCanTestFrame, 7> const CAN_TEST_FRAME_ARRAY =
{
  test_frame_1,
  test_frame_2,
  test_frame_3,
  test_frame_4,
  test_frame_5,
  test_frame_6,
  test_frame_7
};

void IRAM_ATTR handleCANInterrupt()
{
  if(!canInited)
      return;

  mcp2515.onExternalEventHandler();
}

void canBus_setup(int8_t rxPin, int8_t txPin, uint16_t speed)
{
  /* Setup SPI access */
  SPI.begin();

  pinMode(MKRCAN_MCP2515_CS_PIN, OUTPUT);
  digitalWrite(MKRCAN_MCP2515_CS_PIN, HIGH);

  /* Attach interrupt handler to register MCP2515 signaled by taking INT low */
  pinMode(MKRCAN_MCP2515_INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(MKRCAN_MCP2515_INT_PIN), handleCANInterrupt, FALLING);

  mcp2515.begin();

  if(!mcp2515.setLoopbackMode()) // test if we can write something to the MCP2515 (is a device connected?)
  {
    SPI.end();
    mqttSerial.println("CAN-Bus init failed!");
    return;
  }

  canInited = true;

  mcp2515.setBitRate(CanBitRate::BR_20kBPS_12MHZ); // CAN bit rate and MCP2515 clock speed

  std::for_each(CAN_TEST_FRAME_ARRAY.cbegin(),
                CAN_TEST_FRAME_ARRAY.cend(),
                [](sCanTestFrame const frame)
                {
                  if(!mcp2515.transmit(frame.id, frame.data, frame.len)) {
                    Serial.println("ERROR TX");
                  }
                  delay(10);
                });

  mcp2515.setListenOnlyMode();

  mqttSerial.println("CAN-Bus inited");

  /*
  CAN.setPins(rxPin, txPin);
  if(!CAN.begin(speed * 1000))
  {
    mqttSerial.println("CAN-Bus inited FAILED!!");
    return;
  }*/
/*
  // Konfiguration des CAN-Controllers
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)txPin, (gpio_num_t)rxPin, TWAI_MODE_NORMAL);
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
  twai_timing_config_t t_config;

  switch (speed)
  {
  case 1:
    t_config = TWAI_TIMING_CONFIG_1KBITS();
    break;

  case 5:
    t_config = TWAI_TIMING_CONFIG_5KBITS();
    break;

  case 10:
    t_config = TWAI_TIMING_CONFIG_10KBITS();
    break;

  case 12:
    t_config = TWAI_TIMING_CONFIG_12_5KBITS();
    break;

  case 16:
    t_config = TWAI_TIMING_CONFIG_16KBITS();
    break;

  case 20:
    t_config = TWAI_TIMING_CONFIG_20KBITS();
    break;

  case 25:
    t_config = TWAI_TIMING_CONFIG_25KBITS();
    break;

  case 50:
    t_config = TWAI_TIMING_CONFIG_50KBITS();
    break;

  case 100:
    t_config = TWAI_TIMING_CONFIG_100KBITS();
    break;

  case 125:
    t_config = TWAI_TIMING_CONFIG_125KBITS();
    break;

  case 250:
    t_config = TWAI_TIMING_CONFIG_250KBITS();
    break;

  case 500:
    t_config = TWAI_TIMING_CONFIG_500KBITS();
    break;

  case 800:
    t_config = TWAI_TIMING_CONFIG_800KBITS();
    break;

  case 1000:
    t_config = TWAI_TIMING_CONFIG_1MBITS();
    break;

  default:
    mqttSerial.println("CAN-Bus init failed! E1");
    return; // error - wrong speed
    break;
  }

  int result;

  // Initialisierung des CAN-Controllers
  if ((result = twai_driver_install(&g_config, &t_config, &f_config)) != ESP_OK)
  {
    mqttSerial.print("CAN-Bus init failed! E2 - ");
    mqttSerial.println(result);
    return;
  }

  if ((result = twai_start()) != ESP_OK)
  {
    mqttSerial.print("CAN-Bus init failed! E3 - ");
    mqttSerial.println(result);
    return;
  }

  */
}

ulong lastCANReading;

void onReceiveBufferFull(uint32_t const timestamp_us, uint32_t const id, uint8_t const * data, uint8_t const len)
{
  if(!canInited)
    return;

  Serial.print("CAN [ ");
  Serial.print(timestamp_us);
  Serial.print("] ");

  Serial.print("ID");
  if(id & MCP2515::CAN_EFF_BITMASK) Serial.print("(EXT)");
  if(id & MCP2515::CAN_RTR_BITMASK) Serial.print("(RTR)");
  Serial.print(" ");
  Serial.print(id, HEX);

  Serial.print(" DATA[");
  Serial.print(len);
  Serial.print("] ");
  std::for_each(data,
                data+len,
                [](uint8_t const elem) {
                  Serial.print(elem, HEX);
                  Serial.print(" ");
                });
  Serial.println();
}

void canBus_loop()
{

/*
  ulong time = millis();
  if(time - lastCANReading > 5000)
  {
    lastCANReading = time;
    mqttSerial.println("Try CAN-Reading...");
  }*/

/*
  twai_message_t message;

  if (twai_receive(&message, 0) != ESP_OK) {
    return; // no messages
  }

  if (!message.rtr)
  {
    mqttSerial.printf("Message from %i recieved:\n", message.identifier);

    for (uint8_t i = 0; i < message.data_length_code; i++)
    {
        mqttSerial.print(message.data[i], HEX);
        mqttSerial.print(" ");
    }

    mqttSerial.println();
  }

  return; // OK
*/
/*
  // try to parse packet
  int packetSize = CAN.parsePacket();

  if (packetSize) {
    // received a packet
    mqttSerial.print("Received ");

    if (CAN.packetExtended()) {
      mqttSerial.print("extended ");
    }

    if (CAN.packetRtr()) {
      // Remote transmission request, packet contains no data
      mqttSerial.print("RTR ");
    }

    mqttSerial.print("packet with id 0x");
    mqttSerial.print(CAN.packetId(), HEX);

    if (CAN.packetRtr()) {
      mqttSerial.print(" and requested length ");
      mqttSerial.println(CAN.packetDlc());
    } else {
      mqttSerial.print(" and length ");
      mqttSerial.println(packetSize);
    }

    // only print packet data for non-RTR packets
    while (CAN.available()) {
      mqttSerial.print(CAN.read(), HEX);
      mqttSerial.print(" ");
    }
    mqttSerial.println();

    mqttSerial.println();
  }*/
}