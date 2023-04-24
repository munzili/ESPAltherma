![](doc/images/logo.png)

<hr/>

<p align="center">
<a href="https://travis-ci.com/raomin/ESPAltherma"><img src="https://app.travis-ci.com/raomin/ESPAltherma.svg?branch=main&status=passed" /></a>
&nbsp;
<img src="https://img.shields.io/github/last-commit/raomin/ESPAltherma?style=for-the-badge" />
&nbsp;
<img src="https://img.shields.io/github/license/raomin/ESPAltherma?style=for-the-badge" />
&nbsp;
<a href="https://github.com/sponsors/raomin/"><img src="https://github.com/raomin/ESPAltherma/blob/main/doc/images/sponsor.png?raw=true"></a>
&nbsp;
<a href="https://www.buymeacoffee.com/raomin" target="_blank"><img src="https://img.shields.io/badge/Buy%20me%20a%20beer-%245-orange?style=for-the-badge&logo=buy-me-a-beer" /></a>
</p>

<hr/>

<p><b>ESPAltherma</b> is a solution to monitor Daikin Altherma / ROTEX / HOVAL Belaria heat pump activity using just Arduino on an <b>ESP32</b> or <b>ESP8266</b> Microcontroller.</p>

_If this project has any value for you, please consider [buying me a 🍺](https://www.buymeacoffee.com/raomin) or even better [sponsoring ESPAltherma](https://github.com/sponsors/raomin/)!. I don't do this for money but it feels good to get some support! Thanks :)_

## Features

  <ul style="list-style-position: inside;">
    <li>Connects with the serial port of Altherma on port X10A.</li>
    <li>Needs just an ESP32, no need for extra hardware. ESP8266 is also supported.</li>
    <li>Queries the Altherma for selected values at defined interval.</li>
    <li>Converts and formalizes all values in a JSON message sent over MQTT.</li>
    <li>Easily integrates with Home Assistant's MQTT auto-discovery.</li>
    <li>Supports update OverTheAir</li>
    <li>Log messages in Serial + MQTT + Screen (m5StickC)</li>
    <li>Optional: can control (on/off) your heat pump.</li>
</ul>

## Preview

![](doc/images/screenshot.png)

# Prerequisites

## Hardware

- A Daikin Altherma or Daikin Altherma based heat pump (ROTEX, HOVAL Belaria...)
- An ESP32 or ESP8266 *I recommend an ESP32, more precisely the M5StickC, it has an integrated display, a magnet, fits well next to the board and is properly isolated. But any ESP32 should work. A support is added for esp8266.*
- 5 pins JST EH 2.5mm connector (or 4 Dupont wires M-F)

## Software

- Platformio

*That's all!*

# Getting started

## Step 1: Uploading the firmware from source code

1. Download the repository folder and open it in PlatformIO.

2. Open a new terminal inside PlatformIO. Select the PlatformIO extension and click inside the "Quick Access" menu on `New Terminal` or press `F1` and select -> `PlatformIO: New Terminal`

3. Enter following command into the terminal to download all configured submodules. This will download the LittleFS ESP-IDF component.

```bash
git submodule update --init --recursive
```

> **_NOTE:_**  If `git` command was nout found, please make sure you installed the git client. It can be downloaded [here](https://git-scm.com/downloads). After installation, please restart VS Code.

4. Optional - If you are using an **M5StickC** (or M5Stack), select the corresponding environment from the status bar:
Click  ![end m5](doc/images/defaultenv.png) and select **env:M5StickC** on the top. The status bar should display ![end m5](doc/images/m5envv.png)
For **M5StickCPlus** select **env:M5StickCPlus**
If you are using an **ESP8266** select the `nodemcuv2` environement.

5. You're ready to go! Connect your ESP32/ESP8266 and click -> Upload! Or press `F1` and select -> `PlatformIO: Upload`

## <a id="section-generate-definition-files"></a>Step 2: Build X10A and CAN definition files

The parameter and command definitions are hold in a template format. To generate a full useable .json file for upload and use on the X10A or CAN bus, please run following command on the terminal (alternatively select each file descriped below inside platformio with a `right mouse click` and select `Run Python File in Terminal`):

```bash
python scripts\build_x10a_commands.py
python scripts\build_can_commands.py
```

You will get some output that shows witch files are generated. You will find those generated files in the `build` folder and a subfolder called `CAN` or `X10A`. This files are ready for upload and use.

## Step 3: Configure/Setup your ESPAltherma

By default, ESPAltherma will generate an open WiFi network named `ESPAltherma-Config-WiFi`. Connect to the WiFi network, open your browser, and enter the following URL to access the configuration page: `http://192.168.1.1/`.
![](doc/images/webUIConfigPage.png)

Take the changes you need an click on `SAVE CONFIG` on the bottom. If all fields are field with valid input, the config will be sendet to ESPAltherma. This will restart ESPAltherma with the new config. If some fields are not contain valid informations, the saving will be canceld and the bad field marked with a red flag. To get a better description and overview of the options, here is a brief list and description of them:

<details>
<summary><a id="config-section-network"></a>Network</h3></summary>

![](doc/images/webUINetwork.png)

Here you can choose if ESPAltherma should generate an own WiFi signal or connect to a router.

When `Use standalone WiFi` is disabled, near Wifi in range can be scanned by clicking on the blue arrow button to the right. The process will be done in background from ESPAltherma and reported back when finished. During this process, the generated WiFi signal from ESPAltherma will be disabled and enabled again when the process is finished. Please make sure to connect again to the signal, if not done automaticaly, to recieve the scan results and get them dispaly in the dropdown field.

By default, ESPAltherma will use DHCP. You can assign a fixed IP and DNS Server by checking the option `WIFI use static IP`
</details>

<details>
<summary><a id="config-section-mqtt"></a>MQTT</summary>

![](doc/images/webUIMQTT.png)

ESPAltherma needs a MQTT-Broker to commmunicate with external devices and services - its one of its core functions. Please enter the Server, Username and Passwort information. If you enter `localhost` or `127.0.0.1` as server name, this will generated an internal loopback service and no communication over MQTT will be done. This options are more for testing and debug then to use in productiv enviroment.

You can also enter the topic name ESPAltherma will use. All transmitions from ESPAltherma will use this topic name as the root element and subscription will be also done this way for setting values like Ports, Relays, Bus-Calls, ...

The Frequency defines how often MQTT messages with current values will be transmitted over the MQTT-Broker
</details>

<details>
<summary><a id="config-section-pins"></a>Pins</summary>

![](doc/images/webUIPins1.png)

ESPAltherma supports multiple services that can be connected to it over the GPIO's. Those services are:
- [ESPAltherma reset pin](#espaltherma-reset-pin)
- [X10A-Bus](#config-section-x10a)
- [Heating and cooling control over pins](#heating-and-cooling-control)
- [SmartGrid control over pins](#smartgrid-control)
- [CAN-Bus](#config-section-can)

#### ESPAltherma reset pin

When the selected pin gets connected to ground, the current configuration will be changed. ESPAltherma will be restarted with Standalone WiFi enabled. This change will be permanent until config is updated again.

####  Heating and Cooling control

When enabled, the selected pin for heating and cooling will be setted to HIGH or LOW over MQTT. To set the pin a MQTT publish needs to be done on the configured MQTT root topic and `SET/HEATING` or `SET/COOLING` with payload `ON` or `OFF`.

As example: `espaltherma/SET/HEATING` with payload `ON` will enable heating pin.

More information will you find [here](#section-controling-heatpump)

####  SmartGrid control

The selected pins can be setted to HIGH or LOW over MQTT. To set the pin a MQTT publish neds to be done on the configured MQTT root topic and `SET/SG` with payload `0` to `3`.

As example: `espaltherma/SET/SG` with payload `3` will enable SmartGrid Mode `Force ON`.

More information will you find [here](#section-controling-smartgrid)
</details>

<details>
<summary><a id="config-section-x10a"></a>X10A</summary>

![](doc/images/webUIX10A.png)

On the pins section, select your RX TX GPIO pins connected to the X10A port. *The ESP32 has 3 serial ports. The first one, Serial0 is reserved for ESP<-USB->PC communication and ESP Altherma uses the Serial0 for logging (as any other project would do). So if you open the serial monitor on your PC, you'll see some debug from ESPAltherma. ESP32 can map any GPIO to the serial ports. Do NOT use the main Serial0 GPIOs RX0/TX0.* * The ESP8266 only has 1.5 Serial ports so it uses a software based serial driver. You can choose any pins, but some will stop you from being able to use the console*

For ESP32 try to stick to the RX2/TX2 of your board (probably GPIO16/GPIO17). **For M5StickC or M5StickCPlus, 26 and 36 will automatically be used if you selected the corresponding environment**. For ESP8266 pins 4 & 5 (D2 & D1 on the NodeMCUv2) are known to work well.

To start the communication you need to upload a parameters file and select commands from it or define custom commands to be used, that will be readed and transfered over MQTT.

By default the `Protocol I` is selected to be used. Some older heat pumps use a different protocol (`Protocol S`). A closer description, witch protocol needs to be used, may be found inside the parametes file of the model.

1. Under `Upload Parameters file` select the parameters file to upload and click the `Upload` button. The files needs to be generated (see [here](#section-generate-definition-files)). Select a file under the folder `build/X10A/` for upload. Each known model has an own parameters file in .json format. The default language available is `English` but other translation exists too. If a translation for you language exists, you will find it in the corresponding folder.
<br/><br/>
*If you're not sure which one to take, choose the closest or Default.h. The only thing that could happen is that you would have missing values, null values or wrong label names.*

2. After upload the dropdown-list next to the Upload section will be filled with new options. Select the `Model`, `Language` and a `Preset`. The Preset `All` will list available parameters in the selected file. Other presets will filter the List based on definition inside the uploaded file. Once a Preset is selected, a list of availbable commands will pop up to the side (`Parameters` List). Select the parameters needed to be scanned and transmitted by click on them with the left mouse and confirm the selection by clicking the button `Add to selected parameters`. The choosen parameters will be shown now on the right side (`Selected Parameters` List).
<br/>
A wiki page is available [here](https://github.com/raomin/ESPAltherma/wiki/Information-about-Values) where everyone can comment on the values and their definition.

3. Optional - To test the parametes or see there values, you can start fetch them once and see a live result. This allows to test the parameters file and the parameters defined in in. Todo so, click the button `Load model values` on one of the lists.

4. Optional - You can add custom parameters to if a parameter is missing in a certain parametes file. You can also combine files by select one, add the needs parameters, switch to the next parameters file and add the parameters either.

5. You're ready to go! Connect your ESP32/ESP8266 and you should see MQTT messages
</details>

<details>
<summary><a id="config-section-can"></a>CAN-Bus</summary>

![](doc/images/webUIPins2.png)

ESPAltherma supports control over the CAN-Bus of the heatpump. Reading values and sending of commands are supported. To support a CAN-Bus (TWAI) protocol a external controller or transciever needs to be used. Either needs the controller or transciever support the selected kbps for the CAN-Bus. Supported chips are at the moment build in `SJA1000`(*please be aware of the different capatilities of your ESP32-Board and the revision*), `MCP2515` and `ELM327`.

Please select the corrosponding chip and pins under the `Pins` config section.

![](doc/images/webUICAN.png)

Once CAN-Bus is enabled under the `Pins` config section, the section `CAN-Bus` will be available. You can choose the CAN-Bus speed of your heat pump (by default 20 kbps) and enter a MQTT topic name where the values will be published (MQTT root topic name plus the entered CAN topic name).

The poll mode allows to select, if and how values will be fetched from CAN-Bus. If you want to get each X seconds the selected commands values, enable it by select `Auto` and enter a `Poll time in seconds`.


1. Under `Upload CAN-Bus definition file` select the commands file to upload and click the `Upload` button. The files needs to be generated (see [here](#section-generate-definition-files)). Select a file under the folder `build/CAN/` for upload. Each known model has an own commands file in .json format. The default language available is `English` but other translation exists too. If a translation for you language exists, you will find it in the corresponding folder.

2. After upload the dropdown-list next to the Upload section will be filled with new options. Select a `Model` and `Language` from the dropdown. Once selected, a list of available commands will pop up to the side (`Available Commands:` List). Select the commands needed to be scanned and transmitted by click on them with the left mouse and confirm the selection by clicking the button `Add to selected commands`. The choosen commands will be shown now on the right side (`Selected Commands` List).

3. Optional - To test the parametes or see there values, you can start fetch them once and see a live result. This allows to test the commands file and the commands defined in in. Todo so, click the button `Load model values` on one of the lists.

4. Optional - to send commands over CAN, you need to publish it over MQTT. Generate your message with MQTT root topic name plus `SET/` plus the entered CAN topic name plus the `Name` of the command. The payload will be transfered over CAN and converted to the correct bytes based on the datatyp of the given command.

Example to enable `Quiet Mode`: `espaltherma/SET/CAN/quiet_mode` with payload `1`

5. Optional - You can add custom commands to if a parameter is missing in a certain parametes file. You can also combine files by select one, add the needs commands, switch to the next commands file and add the commands either.

6. You're ready to go! Connect your ESP32/ESP8266 and you should see CAN messages
</details>

<details>
<summary><a id="config-section-export-import"></a>Export/Import</summary>

![](doc/images/webUIExport_Import.png)

Here you can save your current config and import an already stored config file. All configuration, including commands, will be exported and restored.
</details>

<details>
<summary><a id="config-section-update"></a>Update</summary>

![](doc/images/webUIUpdate.png)

If a new Firmware is released, the compiled binary can be flashed by select it click `Start update` button
</details>

<details>
<summary><a id="config-section-web-serial"></a>Web Serial</summary>

<img src="doc/images/webUIWebSerial.png" alt="drawing" width="500"/>

This view shows debug and test informations. It allows to set the status of ESPAltherma. Also will you get the `CAN-Sniffing` results if enabled.
</details>

## <a id="section-connect-heatpump"></a>Step 4: Connecting to the Heat pump

1. Turn OFF your heat pump at the circuit breaker.
2. Unscrew your pannel to access the main PCB of your unit.
3. Localize the X10A connector on your the PCB. This is the serial port on the main PCB. If your installation include a bi-zone module, the X10A port is occupied with a connector to the Bi-Zone module. You should then connect to the X12A port on the bi-zone module. Pins are identical to the X10A.
4. Using the 5 pin connector or 4 Dupont wires, connect the ESP as follow. Pay attention to the orientation of the socket.

### Daikin Altherma 4 pin X10A Connection

![The X10A connector](doc/images/schematics.png)

| X10A | ESP32 |
| ---- | ----- |
| 1-5V | 5V - VIN *Can supply voltage for the ESP :)* |
| 2-TX | `RX_PIN` *Default GPIO 16. Prefer RX2 of your board.* |
| 3-RX | `TX_PIN` *Default GPIO 17. Prefer TX2 of your board.* |
| 4-NC | Not connected |
| 5-GND | GND |

> ESP `RX_PIN` `TX_PIN` can be changed in `src/setup.h`.

### 8 pin X10A Connection

Some heat pumps (ROTEX) have an X10A port which connects differently:

![](doc/images/rotexX10A.png)

Some users reported that the 5V from their ROTEX was not enough to power their ESP32/ESP8266. In this case, use an USB charger to power the ESP32/ESP8266. The 5V from the X10A is then not needed. Whatever you do, **make sure you keep a wire connecting the GND of the ESP32/ESP8266 to the GND pin of the X10A (even if you power your ESP32/ESP8266 with a USB charger)!!**


5. Cross check twice the connections and turn on your heat pump. Two new entities AlthermaSensor and AlthermaSwitch should appear in Home Assistant. AlthermaSensor holds the values as attributes.

You can also monitor values and debug messages on your MQTT server:

```bash
$ mosquitto_sub -v -t "espaltherma/#"
```

or via Home Assistant Configuration->Integration->MQTT Configure->Listen to topic espaltherma/# -> Start Listening

## <a id="section-controling-heatpump"></a>Step 5 (optional) - Controling your Daikin Altherma heat pump

ESPAltherma cannot change the configuration values of the heat pump (see [FAQ](#faq)). However, ESPAltherma can control a relay on MQTT that can simulate an *external On Off thermostat*. Doing so allows to remotely turn on/off the heating function of your heat pump. A second relay can be used to trigger the cooling function.

Refer to the schematic map of your heat pump to see where to connect *external On Off thermostat*.

Adding this will take priority on your thermostat. ESPAltherma will turn the heating on/off ; the thermostat will be in standby.

Note: I resoldered the J1 jumper that was cut when installing my digital thermostat (not sure if it is needed) and configured my *type of thermostat* as *External thermostat*

Once installed the setup looks like this:

![](doc/images/installation.png)

Other users installations are described [in this issue](/../../issues/17).

On a Rotex this would connect to J16 Pin 1 and 2. Note: RT needs to be switched ON in the heatpump Connection menu. Heating will be ON if pins are connected, else no heating, so connect to the NC (normally closed) of the relay.

## <a id="section-controling-smartgrid"></a>Step 6 (optional) - Smart grid features
ESPaltherma can also integrate with SG-Ready options of your heat pump. To do so, uncomment and configure `PIN_SG1` and `PIN_SG2` in `src/setup.c` and send one of the allowed values (0..3) to MQTT channel `espaltherma/sg/set`. Current SG mode will be available in `espaltherma/sg/state`.

Of course, you will need to use 2 more relays to open/close SG1 and SG2 contacts of your heat pump.

I found that using 5V supply pin of X10A provides enough power for my ESP32 and both relays, but your mileage may vary.

On a Rotex SG1 and SG2 contacts are located in J8 connector, pin 5-6 (Smart Grid) and 11-12 (EVU) respectively.

Once configured and connected, your heat pump will work like this:

| sg/set value| SG1   | SG2   | SG-Mode              | Working mode | Typical result |
| ----------- | ----- | ----- | -------------------- | ------------ | -------------- |
| 0           | open  | open  | 0 - normal operation | normal working mode        | HP works like if SG features are disabled/not used |
| 1           | open  | close | 1 - Forced OFF       | Hp is forced OFF           | Heating and DHW will be turned OFF - *Beware that your comfort may be negatively affected by this working mode* |
| 2           | close | open  | 2 - Recommended ON   | Hp is recommended to be ON | HP will increase DHW setpoint as well as LW setpoint (documentation says +5 °C, but my tests actually show +6 °C) |
| 3           | close | close | 3 - Force ON         | Hp is forced ON            | HP will increase DHW setpoint and will use its full power to heat DHW (to 70 °C) |

*Note that In SG3 mode your HP will really be power hungry so make sure to enable it only when electricity cost is low (ideally free) or be prepared to get a high bill!*

Depending on your HP model, SG3 might be configurable in "ECO mode", "Normal mode" or "Comfort mode". The mode can be set using the specialist code Main Menu > Settings > Input/Output.

| SG-Mode | Description |
| ------- | ----------- |
| Comfort mode | Increase of the hot water set temperature by 5 K. |
| Normal mode | Increase of flow set temperature by 2 K and hot water set temperature by 5 K. |
| ECO mode | Increase the flow set temperature by 5 K and hot water set temperature by 7 K. |

Note: Smart Grid needs to be switched ON in the heatpump configuration menu, otherwise SG1 and SG2 contacts are not evaluated.

# Troubleshooting

## Specific issues with M5

- If, when using an M5StickC (or M5Stack), the ESP32 is unresponsive, upload fails etc. Make sure that you change the ![default env on pio](doc/images/defaultenv.png) environment to ![end m5](doc/images/m5envv.png) on the status bar. Otherwise the default serial port in setup.h conflicts with the PSRAM of M5.

## Generic issues

Possible generic issues could be: improper wifi signal, unsupported protocol, unsupported GPIOs for Serial (stick to default RX2/TX2).

ESPAltherma generates logs on the main serial port (USB) and on the screen of the M5. Connect to the ESP32 and open the serial monitor on Platformio.

ESPAltherma also generates logs on MQTT. If Wifi and MQTT is not the issue, look at the logs on the topic `espaltherma/log`. You can see them on Home Assistant through  Configuration -> Integration -> MQTT -> Config -> Listen to a topic.

## Logs show 'Timeout on register' with value 0x15 0xea or 'Error 0x15 0xEA returned from HP'

`0x15 0xea` is the reply from the heatpump to say it does not understand the protocol.
If you have an older Altherma heat pump (around 2010 or before) it is probably using the older S protocol.
To activate it, at the end of `setup.h` change `#define PROTOCOL 'I'` to `#define PROTOCOL 'S'`
Also select the `def/PROTOCOL_S_ROTEX.h` or `def/PROTOCOL_S.h` definition file.

## Logs show 'Time out! Check connection' 'Wrong CRC on registry...'

This means that the communication is wrong. Usual suspects:

1. Un-conected GND: whatever you do, the GND of the ESP should always be connected to the GND of the Altherma. So, if you power your ESP with a USB charger (or your computer), make sure you also connect the GND from the ESP to your GND of the Altherma.
2. If not GND, then it's alway the Dupont cable. A faulty dupont cable is a VERY COMMON cause of issue. You can have a perfectly looking cable, they are not the best to do connection on the X10A connector (although much more common than an EH JST 5pin). So, change your cable. You can also use a common 2.54 female long header, plug it to the X10A connector and then your dupont cable to the long pins of the header.
![pic of header](doc/images/header.png)

## Note on voltage

The serial port of X10A is TTL 5V, where the ESP32 is 3.3V. Your ESP32 might not be 5V tolerant. If you want to play it safe, you should use a level shifter to convert Daikin TX - RX ESP line from 5V to 3.3V.

In practice, I had no problem connecting an ESP32 without level shifters. I also had no issue powering the ESP32 from the 5V line of the X10A. On my Daikin Altherma, 5V is provided by a 7805 with a massive heat sink, plus, there are not many clients for it on the board and the ESPAltherma running on my ESP32 consumes 70ma.

Some users reported that a ROTEX did not have a stable 5v that could be used to power the ESP32. If so, you would need to rely on an external 5V power supply (eg a regular USB charger) to power the ESP32.

If you are using an M5StickC you can select the PlatformIO env:m5stickc (or env_m5stickcplus for that version), then ESPAltherna will also report on the voltage and consumption of the M5StickC in the reported values.

# Integrating with Home Assitant

ESPAltherma integrates easily with Home Assistant using [mqtt discovery](https://www.home-assistant.io/docs/mqtt/discovery/).

After setup, ESPAltherma will generate 2 entities on Home Assistant:

![](doc/images/haentities.png)

- `sensor.althermasensors` holds the values as attributes.

- `switch.altherma` activates the relay connected to the `PIN_THERM`

## Declaring sensor entities

In Home Assistant, all values reported by ESPAltherma are `attribute`s of the `entity` sensor.althermasensors.

![](doc/images/attribs.png)

If you want to integrate specific `attribute`s in graphs, gauge etc. you need to declare them as `sensor`s using `template` in your `configuration.yaml`. See [HA doc on Template](https://www.home-assistant.io/integrations/template/).

Eg. this template declares the 2 operation modes as entities:

```yaml
sensor:
  - platform: template
    sensors:
      espaltherma_operation:
        friendly_name: "Operation mode"
        value_template: "{{ state_attr('sensor.althermasensors','Operation Mode') }}"
      espaltherma_iuoperation:
        friendly_name: "Indoor Operation mode"
        value_template: "{{ state_attr('sensor.althermasensors','I/U operation mode') }}"
      espaltherma_dhw:
        friendly_name: "DHW Temp"
        value_template: "{{ state_attr('sensor.althermasensors','DHW tank temp. (R5T)') }}"
        unit_of_measurement: '°C'
```

After restarting Home Assistant, these entities can be added to an history card:

![](doc/images/historycard.png)

## A Climate entity

To control heating through the On/Off switch, declare a Climate (aka thermostat) entity monitoring a temperature sensor.

```yaml
climate:
  - platform: generic_thermostat
    name: Altherma
    heater: switch.altherma
    target_sensor: sensor.temproom1
    min_temp: 15
    max_temp: 25
    cold_tolerance: 0.5
    hot_tolerance: 0.5
    min_cycle_duration:
      minutes: 30
    away_temp: 15
    precision: 0.1
```

Then, add a Thermostat card somewhere:

![ha thermostat](doc/images/thermostat.png)

## Calculating COP

The information returned by ESPAltherma allows to calculate the coefficient of performance (COP). It is the ratio of the heat delivered by your heat pump to the energy consumed by it.

When put in terms of ESPAltherma variables, the COP can be define as a sensor like this in the `sensor:` section of Home Assistant:

```yaml
      espaltherma_cop:
        friendly_name: "COP"
        unit_of_measurement: 'COP'
        value_template: "{% if is_state_attr('sensor.althermasensors','Operation Mode', 'Heating') and is_state_attr('sensor.althermasensors','Freeze Protection', 'OFF')  %}
{{
  ((state_attr('sensor.althermasensors','Flow sensor (l/min)')| float * 0.06 * 1.16 * (state_attr('sensor.althermasensors','Leaving water temp. before BUH (R1T)') | float - state_attr('sensor.althermasensors','Inlet water temp.(R4T)')|float) )
    /
  (state_attr('sensor.althermasensors','INV primary current (A)') | float * state_attr('sensor.althermasensors','Voltage (N-phase) (V)')|float / 1000))
  |round(2)
}}
{% else %} 0 {%endif%}"
```

# FAQ

## Great! I can now monitor my heat pump! Can I change the configuration values too?

Not directly. It might be possible to change registry values using the serial port but I'm not aware of this. If you know, comment on [the dedicated issue](/../../issues/1).

However, ESPAltherma, supports an extra GPIO to control a relay that you can plug as *external On/Off thermostat*. See [**Controling your Daikin Altherma heat pump**](#controling-your-daikin-altherma-heat-pump).

If you want to configure your heat pump using an arduino, you can interact with the P1P2 serial protocol (the one of the digital thermostats) using the [nice work on P1P2Serial](https://github.com/Arnold-n/P1P2Serial) of Arnold Niessen.

## Where can I get more info on the protocol used?

It took quite some time to reverse engineer the protocol. If you're interested, I documented my findings [here](doc/Daikin%20I%20protocol.md).

## Is it safe? Can I break my machine?

It is as safe as interacting with a serial port can be. Pretty safe if you are a bit careful. Use is entirely at your own risk. No liability.

## Why not using the Daikin LAN adapter?

Of course you can probably achieve the same with the BRP069A62 adapter. However, it is expensive, not wifi and less fun than doing it yourself :)

## I selected a value but it is always returning 0 (or OFF)

The definition files contains values for a range of product. It is possible that some of the values are not implemented in your specific heat pump.

If it says 'conv XXX not avail.' it is that I did not implement this specific conversion of value. If you need this value, create an issue and I'll implement it.

## What is the meaning of this value?

Some times the names of the values can be cryptic. Sometimes, the names are more informative on other models: You can look for the registry in other model this can give you a hint. Eg.: One one file `0x62,15` is `"Pressure sensor"` => on the other `0x62,15` is `"Refrigerant pressure sensor"`.

I'm not an expert in heat pump, so I don't understand all possible values. Collectively however, I'm sure that we can understand a lot.

I created [a page in the WIKI](https://github.com/raomin/ESPAltherma/wiki/Information-about-Values). You can add your comments on the register values and suggest possible better names!

## My Daikin heat pump is not an Daikin Altherma. Can I still control it?

No, ESPAltherma supports only Altherma protocol. Other (AC only) units also have a serial port but using other protocols that would require extra reverse engineering to be implemented.

## How can I update ESPAltherma remotely?

Yes! ESPAltherma source code is upgraded often. Your ESPAltherma can be updated Over-The-Air without having to unplug it from the heat pump:

1. Download the updated code from the repository (or pull new changes) and report your configuration.
2. Open platformio.ini and uncomment the following line on your specific environment:

```ini
upload_port = ESPAltherma.local
```

With this parameter, the upload will happen over wifi. Note: your local firewall should allow incoming connection ; also, it can fail from time to time, if it happens just relaunch update.

## I'm using OpenHAB (or others) can I get the values in separated MQTT topics?

Yes, ESPAltherma now supports sending each value to a specific topic in addition to sending a complete JSON on the main topic.
To activate this specific feature uncomment the following lines from `src/setup.h`

```c++
//Uncomment this if you want to activate the One Value <-> One Topic mode. Each value will be sent to a specific topic below
#define ONEVAL_ONETOPIC
#define MQTT_OneTopic "espaltherma/OneATTR/" //Keep the ending "/" !!
```

Now each value will be published in `espaltherma/OneATTR/[valuename]` eg `espaltherma/OneATTR/Boiler Heating Target Temp.`

## How can I contribute?

Every contribution to this project is highly appreciated! Don't fear to create issues to report possible bugs or feature request. Pull requests which enhance or fix ESPAltherma are also greatly appreciated for everybody!

If this project is useful to you, and if you want, <b>[you can buy me a beer](https://www.buymeacoffee.com/raomin)</b>! It feels good and really helps improving ESPAltherma. Thanks :)

You can also [sponsor this project](https://github.com/sponsors/raomin/) (ie regular beers :)) and become an official supporter of ESPAltherma and get your badge on this page!

## ❤ Sponsors ❤

<a href="https://github.com/freddydeschepper">@freddydeschepper</a><br/>
<a href="https://github.com/qris">@qris (Chris Wilson)</a><br/>
<a href="https://github.com/mauromorello">@mauromorello (Mauro Morello)</a><br/>
<a href="https://github.com/kloni">@kloni (Tom Klonikowski)</a><br/>
<a href="https://github.com/tarmor1">@tarmor1</a><br/>
<a href="https://github.com/EvertJob">@EvertJob (toppe)</a><br/>
<a href="https://github.com/FusisCaesar">@FusisCaesar</a><br/>

# License
ESPAltherma is licensed under ![MIT Licence](https://img.shields.io/github/license/raomin/ESPAltherma.svg?style=for-the-badge)
