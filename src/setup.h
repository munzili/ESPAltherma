//Setup your credentials and mqtt info here:
//only change the value between the " " leave the rest of the line untouched.
#define WIFI_SSID "SSID"//**Your SSID here**
#define WIFI_PWD "password"//**Your password here** leave empty if open (bad!)

//Uncomment this to set a static IP instead of DHCP for the ESP (Separate by commas instead of dots)
//#define WIFI_IP 192, 168, 0, 5
//#define WIFI_SUBNET 255, 255, 255, 0
//#define WIFI_GATEWAY 192, 168, 0, 1
//#define WIFI_PRIMARY_DNS 8, 8, 8, 8     //optional
//#define WIFI_SECONDARY_DNS 8, 8, 4, 4   //optional

#define MQTT_SERVER "192.168.1.4"//**IP address here of your MQTT server**
#define MQTT_USERNAME ""//leave empty if not set (bad!)
#define MQTT_PASSWORD ""//leave empty if not set (bad!)
#define MQTT_PORT 1883

#define FREQUENCY 30000 //query values every 30 sec

#if defined(ARDUINO_M5Stick_C) || defined(ARDUINO_M5Stick_C_Plus)
//Values used when **M5StickC** or **M5STickCPlus** environment is selected:
#define RX_PIN    36// Pin connected to the TX pin of X10A 
#define TX_PIN    26// Pin connected to the RX pin of X10A
#else 
//Default GPIO PINs for Serial2:
#define RX_PIN    16// Pin connected to the TX pin of X10A 
#define TX_PIN    17// Pin connected to the RX pin of X10A
#endif

//Uncomment if you use the CAN interface of the Altherma/Rotex heatpump. A external CAN transceiver modul
//is needed. The transceiver modul needs to be connected to the CAN Bus of the heat pump (J13) and ESPAltherma
//#define CAN_ENABLED
//#define CAN_CTX_PIN   25 // Pin connected to the TX pin of CAN transceiver
//#define CAN_CRX_PIN   26 // Pin connected to the RX pin of CAN transceiver

#if defined(ARDUINO_M5Stick_C) || defined(ARDUINO_M5Stick_C_Plus)
//#define PIN_RT_HEATING 0// Pin connected to the thermostat heating relay (normally open)
//#define PIN_RT_COOLING 0// Pin connected to the thermostat heating relay (normally open)
#else
//#define PIN_RT_HEATING 1// Pin connected to the thermostat heating relay (normally open)
//#define PIN_RT_COOLING 2// Pin connected to the thermostat heating relay (normally open)
#endif

//Smart grid control - Optional:
//Uncomment and set to enable SG mqtt functions
//#define PIN_SG1 32// Pin connected to dry contact SG 1 relay (normally open)
//#define PIN_SG2 33// Pin connected to dry contact SG 2 relay (normally open)
// Define if your SG relay board is Low or High triggered (signal pins)
// Only uncomment one of them
#define SG_RELAY_HIGH_TRIGGER
//#define SG_RELAY_LOW_TRIGGER

// DO NOT CHANGE: Defines the SG active/inactive relay states, according to the definition of the trigger status
#if defined(SG_RELAY_LOW_TRIGGER)
#define SG_RELAY_ACTIVE_STATE LOW
#define SG_RELAY_INACTIVE_STATE HIGH
#else
#define SG_RELAY_ACTIVE_STATE HIGH
#define SG_RELAY_INACTIVE_STATE LOW
#endif

#define MAX_MSG_SIZE 4096//max size of the json message sent in mqtt 

//Uncomment this line if the JSON message should be in a Json Table format []. Use only for IOBroker Vis. 
//#define JSONTABLE

//Uncomment this if you want to activate the One Value <-> One Topic mode. Each value will be sent to a specific topic below 
// #define ONEVAL_ONETOPIC
// #define MQTT_OneTopic "espaltherma/OneATTR/" //Keep the ending "/" !!