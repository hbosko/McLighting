//Disable DEBUG on final product
#if ENABLE_DEBUG_service == 1
  #define ENABLE_DEBUG
#endif

#if PASSWD_PROTECTED_service == 1
  #define PASSWD_PROTECTED
#endif

#if ENABLE_OTA_service == 1
  #define ENABLE_OTA
#endif

#if ENABLE_webSockets_service == 1
  #define ENABLE_webSockets
#endif

#if FIRMWARE_UPDATE_service == 1
  #define FIRMWARE_UPDATE
#endif

#if ENABLE_MQTT_service == 1
  #define ENABLE_MQTT
#endif

#if ENABLE_MDNS_service == 1
  #define ENABLE_MDNS
#endif

#if ENABLE_FS_Editor_service == 1
  #define ENABLE_FS_Editor
#endif

#if ENABLE_BLINK_service == 1
  #define ENABLE_BLINK
#endif

#if ENABLE_UDP_service == 1
  #define ENABLE_UDP
#endif

#ifdef ENABLE_DEBUG
  #define DEBUG_PRINT(x)       Serial.print(x)
  #define DEBUG_PRINTDEC(x)    Serial.print(x, DEC)
  #define DEBUG_PRINTLN(x)     Serial.println(x)
  #define DEBUG_PRINTF(x, ...) Serial.printf(x, ##__VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTDEC(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(x, ...)
#endif

