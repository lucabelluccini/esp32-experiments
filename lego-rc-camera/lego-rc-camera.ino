// WARNING!!! Make sure that you have either selected ESP32 Wrover Module or another board which has PSRAM enabled

// SERIAL
#define USE_SERIAL Serial

// CAMERA
#include "esp_camera.h"
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"
void startCameraServer();

// WIFI
#include <WiFi.h>
const char* ssid = "SSID";
const char* password = "PASSWORD";

// WEBSOCKETS
#include <WebSocketsServer.h>
WebSocketsServer webSocket = WebSocketsServer(8080);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        webSocket.sendTXT(num, "Connected");
            }
            break;
        case WStype_TEXT:
            USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);
            translateToPowerFunctions(payload);
            // send message to client
            // webSocket.sendTXT(num, "message here");
            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            break;
        case WStype_BIN:
            USE_SERIAL.printf("[%u] get binary length: %u\n", num, length);
            // send message to client
            // webSocket.sendBIN(num, payload, length);
            break;
    case WStype_ERROR:      
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      break;
    }
}


// LEGO POWER FUNCTIONS

// -*- mode:c++; mode: flymake -*-
#include <PowerFunctions.h>
PowerFunctions pf(12, 0);
uint8_t toPwmSpeedSteps(int value) {
  switch(value) {
    case -7: return B1001;
    case -6: return B1010;
    case -5: return B1011;
    case -4: return B1100;
    case -3: return B1101;
    case -2: return B1110;
    case -1: return B1111;
    case 0:  return B1000;
    case 1:  return B0001;
    case 2:  return B0010;
    case 3:  return B0011;
    case 4:  return B0100;
    case 5:  return B0101;
    case 6:  return B0110;
    case 7:  return B0111;
    default: return B1000;
  }
}

void translateToPowerFunctions(const uint8_t * inputData) {
  static int left = 0;
  static int right = 0;
  if (sscanf((char *)inputData, "%d,%d", &left, &right) != 2) {
    left = 0;
    right = 0;
  }
  USE_SERIAL.printf("LegoPwC - Sending [Left: %d] [Right: %d]\n", left, right);
  //pf.combo_pwm(toPwmSpeedSteps(left), toPwmSpeedSteps(-right));
  pf.combo_pwm(toPwmSpeedSteps(right), toPwmSpeedSteps(-left));
  delay(5);
}

void setup() {

  // Serial setup
  USE_SERIAL.begin(115200);
  USE_SERIAL.setDebugOutput(true);
  USE_SERIAL.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    USE_SERIAL.printf("Camera - Init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  //initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);//flip it back
    s->set_brightness(s, 1);//up the blightness just a bit
    s->set_saturation(s, -2);//lower the saturation
  }
  //drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  // Setup Wifi
  USE_SERIAL.printf("WiFi - Attempting to connect Wifi with SSID '%s'", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    USE_SERIAL.print(".");
  }
  USE_SERIAL.println("\nWiFi - Connected!");

  // Setup Camera
  startCameraServer();

  // Setup websocket
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  USE_SERIAL.printf("Server - Ready. Use 'http://%s' to connect to the Web UI", WiFi.localIP().toString().c_str());
}

void loop() {
  // Websocket loop
  webSocket.loop();
}
