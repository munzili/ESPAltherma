#include "MCP2515.hpp"

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


ulong lastCANReading;

void onReceiveBufferFull(uint32_t const timestamp_us, uint32_t const id, uint8_t const * data, uint8_t const len)
{
    if(!canInited)
        return;

    char resultText[128] = "";
    sprintf(resultText, "CAN [ %i ] ID", timestamp_us);

    if(id & MCP2515::CAN_EFF_BITMASK) strcat(resultText, "(EXT)");
    if(id & MCP2515::CAN_RTR_BITMASK) strcat(resultText, "(RTR)");

    sprintf(resultText + strlen(resultText), " %02X DATA[%i] ", id, len);

    std::for_each(data,
                    data+len,
                    [resultText](uint8_t const elem) mutable {
                        sprintf(resultText + strlen(resultText), "%02X ", elem);
                    });

    mqttSerial.println(resultText);
}

void IRAM_ATTR handleCANInterrupt()
{
  if(!canInited)
      return;

  mcp2515.onExternalEventHandler();
}

bool DriverMCP2515::initInterface()
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
        return false;
    }

    canInited = true;

    mcp2515.setBitRate(CanBitRate::BR_20kBPS_12MHZ); // CAN bit rate and MCP2515 clock speed

    std::for_each(CAN_TEST_FRAME_ARRAY.cbegin(),
                    CAN_TEST_FRAME_ARRAY.cend(),
                    [](sCanTestFrame const frame)
                    {
                    if(!mcp2515.transmit(frame.id, frame.data, frame.len)) {
                        mqttSerial.println("ERROR TX");
                    }
                    delay(10);
                    });

    mcp2515.setListenOnlyMode();

    mqttSerial.println("CAN-Bus inited");

    return true;
}

const char *DriverMCP2515::sendCommandWithID(CommandDef* cmd, bool setValue, int value)
{
}