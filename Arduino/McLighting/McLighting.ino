// disable DEGUG on live product!!
#define ENABLE_DEBUG_service 0

#define ENABLE_webSockets_service 1
#define ENABLE_MQTT_service 1
#define ENABLE_OTA_service 0
#define ENABLE_MDNS_service 1
#define ENABLE_UDP_service 1

#define ENABLE_FS_Editor_service 1
#define FIRMWARE_UPDATE_service 0 // too lazy to finishh...

// 1 = progmem, 0 = spiffs (upload index.html.gz)
#define include_indexHTML 1

#define PASSWD_PROTECTED_service 0

//builtInLed // disable (0) if DATA pin = 2
#define ENABLE_BLINK_service 0

#if include_indexHTML == 1
  #include "index_HTML.h"
#endif

#include "0_define.h"
#include "0_ESP_settings.h"
#include "1_library.h"
#include "prototype.h"
#ifdef ENABLE_UDP
  #include "UDP_indexHTML.h"
  #include "UDP_cs.h"
#endif
#include "enum_stuff.h"
#include "0_helpers.h"
#include "SVC_FS_editor.h"
#include "SVC_MQTT.h"
#include "SVC_OTA.h"
#include "SVC_Scanner.h"
#include "MOD_esp_pin.h"
#include "WManagerCustom.h"

//  #include "palette.h" // fastled
  #include "palette_custom.h" //ws2812 
  #include "palette_built_in.h" //ws2812 
#include "pixel_settings.h"
#include "strip_state.h"
#include "strip_config.h"
//#include "pixel_config.h"
#ifdef ENABLE_LEGACY_ANIMATIONS
  #include "colormodes.h"
#endif

void setup() {
  #ifdef ENABLE_DEBUG
    Serial.begin(115200);
    DEBUG_PRINTLN("Debugging enabled!");
    DEBUG_PRINTLN("-----");
  #endif

  init_ChipID();

  #if defined(ESP8266)
    if(!LittleFS.begin()){ DEBUG_PRINTLN("=<=< Error LittleFS"); } 
    else { DEBUG_PRINTLN("=> ESP8266 LittleFS mounted successfully"); }
  #elif defined(ESP32)
    if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){ DEBUG_PRINTLN("=<=< Error LittleFS"); return; } 
    else { DEBUG_PRINTLN("=> ESP32 LittleFS mounted successfully"); }
  #endif
  
  init_esp_settings();
  Wire.begin(SDA_pin, SCL_pin);

    #ifdef PASSWD_PROTECTED
      init_global_login();
    #endif

  initWMConfigDefaults();
  init_CUSTOM_WIFI();
  init_firmwareUpdater();
  
  WIFIMANAGER_server_path();
  init_WEB_server();

  #ifdef ENABLE_UDP
    udp_cs_init();
  #endif

  #ifdef ENABLE_BLINK
    init_ESP_PIN();
  #endif

  #ifdef ENABLE_FS_Editor
    init_FS_editor();
  #endif
  #ifdef ENABLE_webSockets
    init_webSockets();
  #endif
  #ifdef ENABLE_MQTT
    init_MQTT_async();
  #endif

    delay(100);
  init_palette_custom_json();
    delay(100);
  init_strip_state_path();
    delay(100);
  init_strip_config_path();
  //read_strip_config_FS();
    delay(100);
  init_Strip();
    delay(50);
}

void loop() {
  loop_CUSTOM_WIFI();

  #ifdef ENABLE_webSockets
    #ifdef ESP8266
      ws.cleanupClients(4);
    #else
      ws.cleanupClients();
    #endif
  #endif

  #ifdef ENABLE_MQTT1
    MQTT_LOOP();
  #endif

  unsigned long currentMillis = millis();

  if (currentMillis - loop_time_1h >= interval_1h) {
    loop_time_1h = currentMillis;
    tokenTimeOut();
  }
  
  if (currentMillis - loop_time_5m >= interval_5m) {
    loop_time_5m = currentMillis;
  }

  if (currentMillis - loop_time_30 >= interval_30s) {
    loop_time_30 = currentMillis;
    notifyClients(mqtt_ESPinfo, ESPinfo());
  }

  switch (Mode) {
    case SET_MODE:
      if (strip) {
        strip->setMode(ws2812fx_mode);
        strip->trigger();
      }
      prevmode = SET_MODE;
      Mode = SETCOLOR;

      //ESP.wdtFeed();
      yield();

      break;

    case OFF:
      if (strip && strip->isRunning()) {
        strip->stop();
      }
      break;

    case SETCOLOR:
      if (strip) {
        strip->setColor(main_color.red, main_color.green, main_color.blue);
        strip->trigger();
      }
      Mode = (prevmode == SET_MODE) ? SETSPEED : HOLD;
      break;

    case SETSPEED:
      if (strip) {
        strip->setSpeed(convertSpeed(ws2812fx_speed));
        strip->trigger();
      }
      Mode = (prevmode == SET_MODE) ? BRIGHTNESS : HOLD;
      break;

    case BRIGHTNESS:
      if (strip) {
        strip->setBrightness(brightness);
        strip->trigger();
      }
      if (prevmode == SET_MODE) prevmode = HOLD;
      Mode = HOLD;
      break;

    default:
      break;
  }

  #ifdef FIRMWARE_UPDATE
    httpUpdate_loop();
  #endif

    handleUdpSync_loop();
  TriggerStripSave_loop();

  if (strip) strip->service();

  #ifdef ENABLE_BLINK
    wifiConnectingLedStatus();
    run_ESP_Blink();
  #endif

  client_state();
  esp_reboot_timer();
}

