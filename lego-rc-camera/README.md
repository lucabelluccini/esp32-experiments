# Lego RC Camera

Remote controlled Lego RC Car with onboard camera (e.g. LEGO 42065 Tracked Racer).

## Hardware requirements

* ESP32-Cam (e.g. Camera module OV2640 / Ai-Thinker, [link](https://www.banggood.com/ESP32-CAM-WiFi-bluetooth-Camera-Module-Development-Board-ESP32-With-Camera-Module-OV2640-p-1394679.html?rmmds=buy&cur_warehouse=CN))
* LEGO 42065 Tracked Racer ([link](https://www.lego.com/en-us/product/rc-tracked-racer-42065))
* An IRLED, a 220Ω resistor
* FTDI to USB serial programmer
* USB Power bank
* A Joypad, compatible with the browser [Gamepad API](https://developer.mozilla.org/en-US/docs/Web/API/Gamepad_API). You can test it [here](https://html5gamepad.com/)

## Schematics

```
                    +------------------------------------------------------------+
                    |                                                            |
                    |    +--------------------------+                            |  +-------------+
                    |    |                          |                            |  |             |
                    +----+ 5V                   3V3 |                            +--+ VCC         |
                         |        ESP32-CAM         |                               |             |
         +---------------+ GND                 IO16 |                      +--------+ RX          |
         |               |                          |                      |        |       FTDI  +----------+ USB(Serial)
         |       +-------+ IO12                 IO0 | <----+               |  +-----+ TX          |
         /       |       |                          |  SC when programming |  |     |             |
         \       |       | IO13                 GND | <----+               |  |  +--+ GND         |
 220Ω    /       |       |                          |                      |  |  |  |             |
         \       |       | IO15                 VCC |                      |  |  |  +-------------+
         |       |       |                          |                      |  |  |
         |       |       | IO14                 U0R +----------------------+  |  |        FTDI Board
         |       |       |                          |                         |  |        set to 5V
        _|_      |       | IO2                  U0T +-------------------------+  |
IR Led _\_/_     |       |                          |                            |
         |       |       | IO4                  GND +----------------------------+
         |       |       |                          |
         +-------+       |                          |
                         +--------------------------+

```

## Software & Libraries

- Download [arduino-cli](https://github.com/arduino/arduino-cli) and add it in your `$PATH`
- Download [github.com/Links2004/arduinoWebSockets](https://github.com/Links2004/arduinoWebSockets) and place them in `~/Arduino/libraries`
- Download [jurriaan/Arduino-PowerFunctions]( https://github.com/jurriaan/Arduino-PowerFunctions) and place them in `~/Arduino/libraries`

### Add support for ESP32 to Arduino

Those steps can be done using the Arduino from the UI:
- `File / Preferences` add the 2 urls, comma separated, in `Additional Boards Manager URLs`
- `Tools / Board / Board Manager` and download ESP32
- Select `AI Thinker ESP32-CAM` from `Tools / Board` 

In case you're using a similar board, you may select the correct board name (on the UI).
This must be aligned when using the CLI. Run `arduino-cli board listall` to get the list of boards and the associated "identifier".
In my case it is `esp32:esp32:esp32cam`.

From CLI:

```
echo << EOF > ~/.cli-config.yml
board_manager:
  additional_urls:
    - http://arduino.esp8266.com/stable/package_esp8266com_index.json
    - https://dl.espressif.com/dl/package_esp32_index.jso
EOF

arduino-cli core update-index
arduino-cli core install esp32:esp32
```

### Clone the project

Clone this project in `~/Arduino/lego-rc-camera/`.

### Tweak the code to use the correct settings.

On the `lego-rc-camera.ino` file, uncomment one of those lines to select the camera you want to use.
In my case `CAMERA_MODEL_AI_THINKER`.
```
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
#define CAMERA_MODEL_AI_THINKER
```

Change the following variables to allow the board to connect to your WiFi.
```
const char* ssid = "yourssid";
const char* password = "yourpassword";
```

### Build the project

From CLI:
```
arduino-cli compile --fqbn esp32:esp32:esp32cam ~/Arduino/lego-rc-camera/
```

From the UI, just build it using `Sketch / Verify/Compile`.

### Set the ESP32-CAM board in flash mode

Shortcircuit `IO0` to `GND` and reset the board using the `RESET` button.

### Flash the sketch to the board

From CLI:
```
# Replace /dev/ttyAMA0 by the serial port of your FTDI board
# On Microsoft Windows, use COMn (e.g. -p COM4 ...)
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32cam ~/Arduino/lego-rc-camera/
```

From the UI, just select `Sketch / Upload`.

### Reset the ESP32-CAM in boot mode

Open the circuit between `IO0` to `GND`. Reset the board.

On the serial output you should get the IP.

### Install your board on the LEGO RC car

Build a structure to install your ESP32-CAM board on the LEGO car.

Make the IR LED point to the original IR Receiver by LEGO.

Install the USB Power bank and connect it to the FTDI board.

Power on the original power pack by LEGO.


