#ifdef ENABLE_OTA

void init_OTA(const char* ota_name) {
  DEBUG_PRINTLN("init OTA enabled!");
  OTA_enabled = 1;

  ArduinoOTA.onStart([]() {
    if (ArduinoOTA.getCommand() == U_FLASH) {
      DEBUG_PRINTLN("Start updating sketch");
    } else {
      DEBUG_PRINTLN("Start updating filesystem");
    }
  });

  ArduinoOTA.onEnd([]() {
    DEBUG_PRINTLN("\nEnd");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    if (total > 0) {
      unsigned int percent = (progress * 100) / total;
      DEBUG_PRINTF("Progress: %u %%\r\n", percent);
    }
  });

  ArduinoOTA.onError([](ota_error_t error) {
    DEBUG_PRINTF("OTA Error [%u]: ", error);

    if (error == OTA_AUTH_ERROR)         DEBUG_PRINTLN("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)   DEBUG_PRINTLN("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) DEBUG_PRINTLN("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) DEBUG_PRINTLN("Receive Failed");
    else if (error == OTA_END_ERROR)     DEBUG_PRINTLN("End Failed");
  });

  ArduinoOTA.setHostname(ota_name);
  ArduinoOTA.begin();
}

#endif

void init_MDNS(const char* md_name) {
  #ifdef ENABLE_MDNS
    MDNS_enabled = 1;
    if (!MDNS.begin(md_name)) {
      DEBUG_PRINTLN("Error setting up MDNS responder!");
      MDNS_connected  = 0;
      return; 
    } else {
      DEBUG_PRINTF("mDNS responder started: %s\r\n", md_name);
      MDNS_connected = 1;
    }
    
    MDNS.addService(MDNS_SERVICE_NAME, MDNS_PROTOCOL, MDNS_PORT);
      MDNS.addServiceTxt(MDNS_SERVICE_NAME, MDNS_PROTOCOL, "brd", ARDUINO_BOARD);
      MDNS.addServiceTxt(MDNS_SERVICE_NAME, MDNS_PROTOCOL, "ver", FIRMWARE_VERSION);
      MDNS.addServiceTxt(MDNS_SERVICE_NAME, MDNS_PROTOCOL, "mac", String(WiFi.macAddress()));
      MDNS.addServiceTxt(MDNS_SERVICE_NAME, MDNS_PROTOCOL, "name", String(device_name));
    #if defined(ESP8266)
      MDNS.addServiceTxt(MDNS_SERVICE_NAME, MDNS_PROTOCOL, "rom", toCleanString(formatBytesToString(ESP.getFlashChipRealSize())));

    #elif defined(ESP32)
      MDNS.addServiceTxt(MDNS_SERVICE_NAME, MDNS_PROTOCOL, "rom", toCleanString(formatBytesToString(ESP.getFlashChipSize())));
    #endif

  #endif
}
