//#define ARDUINOJSON_ENABLE_DEPRECATED 1

#include <Arduino.h>
#include <array>
#include <AsyncJson.h>
#include <ArduinoJson.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
  #include <ESP8266HTTPClient.h>
  #include <ESP8266httpUpdate.h>
  
  WiFiClient client; 
  #define FW_BOARD_TYPE_STR "esp8266"
  #define ESP_REBOOT ESP.reset();

#elif defined(ESP32)
  #include <WiFi.h>
  #include <AsyncTCP.h>
  #include <HTTPClient.h>
  #include <HTTPUpdate.h>
  
  WiFiClient client; 
  #define FW_BOARD_TYPE_STR "esp32"
  #define ESP_REBOOT ESP.restart();
  #define FORMAT_LITTLEFS_IF_FAILED true
  #define LED_BUILTIN 8
#endif

#include <LittleFS.h>

#ifdef ENABLE_OTA
  #include <WiFiUdp.h>
  #include <ArduinoOTA.h>
#endif

  // use this https://github.com/Aircoookie/ESPAsyncWebServer
#include "ESPAsyncWebServer.h"
AsyncWebServer server(www_port);
  String HTTP_client;
    String HTTP_client_last;
  String LOGIN_client;
    String LOGIN_client_last;
  String LOGOUT_client;
    String LOGOUT_client_last;
  String NotFound_client;
    String NotFound_client_last;
  String wifi_edit_client;
    String wifi_edit_client_last;
  String Settings_client;
    String Settings_client_last;
      uint8_t client_loged_out = 0;
      uint8_t client_loged_out_last;
      const char* client_loged = "Client loged: ";

    uint8_t conf_type = 0;
    uint8_t conf_sub_type = 0;
    char custom_type[32];
    char custom_sub_type[32];

  String globalSessionToken = "";

#include <DNSServer.h>
DNSServer dnsServer;

#ifdef ENABLE_webSockets
  AsyncWebSocket ws(ws_path);
  String WS_client;
  String WS_client_last;
#endif

#include "src/ESPAsyncHTTPUpdateServer/ESPAsyncHTTPUpdateServer.h"
  ESPAsyncHTTPUpdateServer updateServer;

#ifdef ENABLE_FS_Editor
  #include "src/SPIFFSEditor_MOD/SPIFFSEditor_MOD.h"
#endif

#ifdef ENABLE_MDNS
  #if defined(ESP8266)
    #include <ESP8266mDNS.h>
  #elif defined(ESP32)
    #include "ESPmDNS.h"
  #endif
#endif

#include "src/AsyncMqttClient/AsyncMqttClient.h"
AsyncMqttClient mqttClient;
String MQTT_client;
String MQTT_client_last;

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <WiFiUdp.h>
  WiFiUDP udp;
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WiFiUDP.h>
  WiFiUDP udp;
#endif

// Strip
// https://github.com/kitesurfer1404/WS2812FX
  //  #include <WS2812FX.h>
  #include "src\WS2812FX\WS2812FX.h"

// WS2812FX strip = WS2812FX(NUMLEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
WS2812FX* strip;

#include <Ticker.h>

Ticker ticker;
Ticker save_strip_config_ticker;
Ticker save_strip_state_ticker;

volatile bool triggerSaveStripConfig = false;
volatile bool triggerSaveStripState  = false;

#include <FastLED.h>  //https://github.com/FastLED/FastLED

Ticker autoTicker;
int autoCount = 0;


#include <Wire.h>
  #include <OneWire.h>
  #include <DallasTemperature.h>
    OneWire oneWire(Dallass_pin);  // on pin X (a 4.7K pullup is necessary or 2k2 on 3.3v)
    DallasTemperature sensors(&oneWire);
