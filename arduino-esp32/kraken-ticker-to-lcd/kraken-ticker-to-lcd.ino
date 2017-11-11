#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
// Requires ArduinoJson http://arduinojson.org/
#include <ArduinoJson.h>

// Requires https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define USE_SERIAL Serial

// Pinout ESP32Wroom Doit V1 https://github.com/playelek/pinout-doit-32devkitv1

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

WiFiMulti wifiMulti;
HTTPClient http;

const char* wifiSsid     = "";
const char* wifiPassword = "";

const char* krakenApiUrl = "https://api.kraken.com/0/public/Ticker?pair=XBTEUR";
const char* krakenCaCert = \ 
"-----BEGIN CERTIFICATE-----\n" \ 
"MIIDnzCCAyWgAwIBAgIQWyXOaQfEJlVm0zkMmalUrTAKBggqhkjOPQQDAzCBhTEL\n" \ 
"MAkGA1UEBhMCR0IxGzAZBgNVBAgTEkdyZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UE\n" \ 
"BxMHU2FsZm9yZDEaMBgGA1UEChMRQ09NT0RPIENBIExpbWl0ZWQxKzApBgNVBAMT\n" \ 
"IkNPTU9ETyBFQ0MgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTQwOTI1MDAw\n" \ 
"MDAwWhcNMjkwOTI0MjM1OTU5WjCBkjELMAkGA1UEBhMCR0IxGzAZBgNVBAgTEkdy\n" \ 
"ZWF0ZXIgTWFuY2hlc3RlcjEQMA4GA1UEBxMHU2FsZm9yZDEaMBgGA1UEChMRQ09N\n" \ 
"T0RPIENBIExpbWl0ZWQxODA2BgNVBAMTL0NPTU9ETyBFQ0MgRG9tYWluIFZhbGlk\n" \ 
"YXRpb24gU2VjdXJlIFNlcnZlciBDQSAyMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcD\n" \ 
"QgAEAjgZgTrJaYRwWQKOqIofMN+83gP8eR06JSxrQSEYgur5PkrkM8wSzypD/A7y\n" \ 
"ZADA4SVQgiTNtkk4DyVHkUikraOCAWYwggFiMB8GA1UdIwQYMBaAFHVxpxlIGbyd\n" \ 
"nepBR9+UxEh3mdN5MB0GA1UdDgQWBBRACWFn8LyDcU/eEggsb9TUK3Y9ljAOBgNV\n" \ 
"HQ8BAf8EBAMCAYYwEgYDVR0TAQH/BAgwBgEB/wIBADAdBgNVHSUEFjAUBggrBgEF\n" \ 
"BQcDAQYIKwYBBQUHAwIwGwYDVR0gBBQwEjAGBgRVHSAAMAgGBmeBDAECATBMBgNV\n" \ 
"HR8ERTBDMEGgP6A9hjtodHRwOi8vY3JsLmNvbW9kb2NhLmNvbS9DT01PRE9FQ0ND\n" \ 
"ZXJ0aWZpY2F0aW9uQXV0aG9yaXR5LmNybDByBggrBgEFBQcBAQRmMGQwOwYIKwYB\n" \ 
"BQUHMAKGL2h0dHA6Ly9jcnQuY29tb2RvY2EuY29tL0NPTU9ET0VDQ0FkZFRydXN0\n" \ 
"Q0EuY3J0MCUGCCsGAQUFBzABhhlodHRwOi8vb2NzcC5jb21vZG9jYTQuY29tMAoG\n" \ 
"CCqGSM49BAMDA2gAMGUCMQCsaEclgBNPE1bAojcJl1pQxOfttGHLKIoKETKm4nHf\n" \ 
"EQGJbwd6IGZrGNC5LkP3Um8CMBKFfI4TZpIEuppFCZRKMGHRSdxv6+ctyYnPHmp8\n" \ 
"7IXOMCVZuoFwNLg0f+cB0eLLUg==\n" \ 
"-----END CERTIFICATE-----\n";

const int refreshIntervalSeconds = 60;

// Get it via http://arduinojson.org/assistant/
const size_t bufferSize = JSON_ARRAY_SIZE(0) + 6*JSON_ARRAY_SIZE(2) + 2*JSON_ARRAY_SIZE(3) + JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(9) + 250;
DynamicJsonBuffer jsonBuffer(bufferSize);

struct KrakenData {
  bool   error;
  bool   parsed;
  String rawData;
  String timestamp;
  double lastValue;
  double lowValue;
  double highValue;
};

KrakenData krakenData;

void setupWifi()
{
  wifiMulti.addAP(wifiSsid, wifiPassword);
}

void setupHttp()
{
  // allow reuse (if server supports it)
  http.setReuse(true);
}

// Generated using https://omerk.github.io/lcdchargen/
byte arrowUp[8] = {
  0b00100,
  0b01110,
  0b11111,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100
};
byte arrowDown[8] = {
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b11111,
  0b01110,
  0b00100
};
#define ARROWUP 0
#define ARROWDOWN 1
void setupLcd()
{
  lcd.begin();
  lcd.backlight();
  lcd.createChar(ARROWUP, arrowUp);
  lcd.createChar(ARROWDOWN, arrowDown);
}

void setup()
{
  USE_SERIAL.begin(115200);
  setupWifi();
  setupHttp();
  setupLcd();
}

bool readDataFromKraken()
{
  krakenData.parsed = false;
  JsonObject& root = jsonBuffer.parseObject(krakenData.rawData.c_str());
  if (root.success() && root.containsKey("result"))
  {
    JsonObject& dataForXBTEUR = root["result"]["XXBTZEUR"];
    krakenData.lastValue = dataForXBTEUR["c"][0].as<double>();
    krakenData.lowValue = dataForXBTEUR["l"][0].as<double>();
    krakenData.highValue = dataForXBTEUR["h"][0].as<double>();
    USE_SERIAL.printf("Timestamp: %s\n", krakenData.timestamp.c_str());
    USE_SERIAL.printf("Last: %5.0f Low: %5.0f High: %5.0f\n", krakenData.lastValue, krakenData.lowValue, krakenData.highValue);
    krakenData.parsed = true;
  }
  return krakenData.parsed;
}

void writeToLcd(const char * line1, const char * line2)
{
  lcd.clear();
  if(line1 != NULL)
  {
    lcd.print(line1);
  }
  if(line2 != NULL)
  {
    lcd.print(line2);
  }
}

void writeToLcd()
{
  lcd.clear();
  size_t dateOnlyOffset(krakenData.timestamp.length()-12);
  lcd.printf("%12s[%c%c]", krakenData.timestamp.substring(dateOnlyOffset).c_str(), (krakenData.error? '!' : ' '), (krakenData.parsed? ' ' : '!'));
  USE_SERIAL.printf("Timestamp shortened: %s\n", krakenData.timestamp.substring(dateOnlyOffset).c_str());
  lcd.setCursor(0,1);
  lcd.printf("%.0f %c%.0f %c%.0f", krakenData.lastValue, (uint8_t)ARROWDOWN, krakenData.lowValue, (uint8_t)ARROWUP, krakenData.highValue);
}

void loop() {
  if((wifiMulti.run() == WL_CONNECTED)) {
    krakenData.error = true;
    krakenData.rawData = "";
    http.begin(krakenApiUrl, krakenCaCert);
    const char * headerKeys[] = {"date", } ;
    size_t headerKeysSize = sizeof(headerKeys)/sizeof(headerKeys[0]);
    http.collectHeaders(headerKeys, headerKeysSize);
    const int httpCode = http.GET();
    if(httpCode > 0) {
      if(httpCode == HTTP_CODE_OK) {
        krakenData.error = false;
        USE_SERIAL.printf("[HTTP] OK\n");
        krakenData.rawData = http.getString();
        USE_SERIAL.printf("Data: %s\n", krakenData.rawData.c_str());
        if(http.hasHeader("date"))
        {
          krakenData.timestamp = http.header("date");
        }
        else
        {
          krakenData.timestamp = "N/A Timestamp";
          USE_SERIAL.printf("Timestamp N/A\n");
        }
        krakenData.error = !readDataFromKraken();
      }
      else
      {
        USE_SERIAL.printf("[HTTP] Error: %s\n", http.errorToString(httpCode).c_str());
        krakenData.error = true;
      }
    } else {
        USE_SERIAL.printf("[HTTP] Error: %s\n", http.errorToString(httpCode).c_str());
    }
    
    writeToLcd();
    
    http.end();
    delay(refreshIntervalSeconds * 1000);
  }
  else
  {
    USE_SERIAL.printf("[WIFI] Connecting...\n");
    writeToLcd("Connecting Wifi", "Please wait...");
    delay(1000);
  }
}



