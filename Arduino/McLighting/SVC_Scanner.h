// Do not request more often than 5 seconds
void scanWifi(Print& output) {
  JsonDocument doc;
  JsonArray networks = doc.to<JsonArray>();

  int n = WiFi.scanComplete();
  
  if (n == -2 || n == -1) {
    WiFi.scanNetworks(true);
    networks.add(F("scanning"));
  } 
  else if (n == 0) {
    networks.add(F("wifiNotFound"));
    WiFi.scanDelete();
    WiFi.scanNetworks(true);
  } 
  else if (n > 0) {
    int indices[n];
    for (int i = 0; i < n; i++) indices[i] = i;

    for (int i = 0; i < n; i++) {
      for (int j = i + 1; j < n; j++) {
        if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
          std::swap(indices[i], indices[j]);
        }
      }
    }

    for (int i = 0; i < n; ++i) {
      int idx = indices[i];
      if (WiFi.SSID(idx).length() == 0) continue;

      JsonObject netObj = networks.add<JsonObject>();
      
      netObj[F("rssi")]    = WiFi.RSSI(idx);
      netObj[F("ql")] = getRSSIasQuality(WiFi.RSSI(idx));
      netObj[F("ssid")]    = WiFi.SSID(idx);
      netObj[F("bssid")]   = WiFi.BSSIDstr(idx);
      netObj[F("ch")] = WiFi.channel(idx);
      netObj[F("secure")]  = WiFi.encryptionType(idx);
      
      #if defined(ESP8266)
        netObj[F("hidden")] = WiFi.isHidden(idx);
      #elif defined(ESP32)
        netObj[F("hidden")] = false;
      #endif
    }
    
    WiFi.scanDelete();
    WiFi.scanNetworks(true);
  }

  serializeJson(doc, output);
}

void scan_i2c(Print& output) {
  JsonDocument doc;
  JsonArray addresses = doc.to<JsonArray>();
  byte error, address;
  bool foundAny = false;
  
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      char hexStr[8];
      snprintf(hexStr, sizeof(hexStr), "0x%02X", address);
      addresses.add(String(hexStr));
      foundAny = true;
    }
    else if (error == 4) {
      char hexStr[16];
      snprintf(hexStr, sizeof(hexStr), "0x%02X_error", address);
      addresses.add(String(hexStr));
      foundAny = true;
    }
    
    delay(5);
  }
  
  if (!foundAny) {
    addresses.add(F("no addresses"));
  }
  
  serializeJson(doc, output);
}
