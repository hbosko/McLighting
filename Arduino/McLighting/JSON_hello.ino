void ESPinfo(String &output) {
  JsonDocument doc;

  JsonObject info = doc[F("info")].to<JsonObject>();
  info[F("ssid")]         = WiFi.SSID();
  info[F("name")]         = device_name;
  info[F("chp_id")]       = chipID_str;
  info[F("ip")]           = WiFi.localIP().toString();
  info[F("mac")]          = WiFi.macAddress();
  info[F("brd")]          = getCleanBoardName();
  info[F("fw")]           = FIRMWARE_VERSION;
  info[F("modul")]        = MODULE_TYPE(conf_type);
  info[F("sub_modul")]    = MODULE_sub_TYPE(conf_sub_type);
  info[F("rssi_ql")]      = getRSSIasQuality(WiFi.RSSI());
  info[F("flash")]        = toCleanString(formatBytesToString(ESP.getFlashChipSize()));
  info[F("up")]           = toCleanString(getUptime());
  info[F("mhz")]          = ESP.getCpuFreqMHz();
  info[F("ram_free")]     = formatBytesToFloat(ESP.getFreeHeap());
  info[F("ram_free_str")] = toCleanString(formatBytesToString(ESP.getFreeHeap()));
  info[F("rssi")]         = WiFi.RSSI();
  info[F("core")]         = ESP.getCoreVersion();
  info[F("json")]         = F(ARDUINOJSON_VERSION);
  info[F("mrbt")]         = manual_reboot_req;

  #if defined(ESP8266)
    info[F("heap_frag")]  = ESP.getHeapFragmentation();
    info[F("SDK")]        = system_get_sdk_version();
    info[F("rst")]        = ESP.getResetReason();
  #elif defined(ESP32)
    info[F("chiprev")]        = ESP.getChipRevision();
    info[F("chipmodel")]      = ESP.getChipModel();
    info[F("chipcores")]      = ESP.getChipCores();
    info[F("heap_total")]     = ESP.getHeapSize();
    info[F("heap_used")]      = ESP.getHeapSize() - ESP.getFreeHeap();
    info[F("heap_max_block")] = ESP.getMaxAllocHeap();
    info[F("heap_min_free")]  = ESP.getMinFreeHeap();
    info[F("SDK")]            = ESP.getSdkVersion();
    info[F("rst")]            = getResetReason_esp32();
    
    if (ESP.getFreeHeap() > 0) {
      info[F("heap_frag")]    = 100 - ((ESP.getMaxAllocHeap() * 100) / ESP.getFreeHeap());
    } else {
      info[F("heap_frag")]    = 100;
    }
  #endif

  JsonObject clientinfo = doc[F("clients")].to<JsonObject>();
  clientinfo[F("HTTP")]      = HTTP_client;
  clientinfo[F("LOGIN")]     = LOGIN_client;
  clientinfo[F("LOGOUT")]    = LOGOUT_client;
  clientinfo[F("WS")]        = WS_client;
  clientinfo[F("BRD")]       = Settings_client;
  clientinfo[F("notfound")]  = NotFound_client;
  clientinfo[F("Wifi_edit")] = wifi_edit_client;
  clientinfo[F("loged_out")] = client_loged_out;

  JsonObject conn = doc[F("conn")].to<JsonObject>();
  conn[F("HTTP")]            = WIFI_connected;
  conn[F("MQTT")]            = MQTT_connected;
  conn[F("MDNS")]            = MDNS_connected;

  JsonObject udp = doc[F("udp")].to<JsonObject>();
  udp[F("conn")]             = UDP_connected;
  udp[F("port")]             = udpPort;
  udp[F("mode")]             = Udp_mode_str;

  JsonObject services = doc[F("services")].to<JsonObject>();
  services[F("HTTP")]        = HTTP_enabled;
  services[F("WS")]          = WS_enabled;
  services[F("MDNS")]        = MDNS_enabled;
  services[F("OTA")]         = OTA_enabled;

  JsonObject modulez = doc[F("modules")].to<JsonObject>();
  modulez[F("I2C")]          = MODULE_I2C;
  modulez[F("SPI")]          = MODULE_SPI;
  modulez[F("Display")]      = MODULE_Display;
  modulez[F("OneWire")]      = MODULE_OneWire;

  serializeJson(doc, output);
}

String ESPinfo() {
  String jsonResponse;
  jsonResponse.reserve(1024);
  ESPinfo(jsonResponse);
  return jsonResponse;
}
