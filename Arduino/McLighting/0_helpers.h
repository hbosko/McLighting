bool esp_reboot = false;

// https://stackoverflow.com/questions/9072320/split-string-into-string-array
String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

#if defined(ESP32)
  String getResetReason_esp32() {
    esp_reset_reason_t reason = esp_reset_reason();
    
    switch (reason) {
      case ESP_RST_UNKNOWN:   return "Unknown";
      case ESP_RST_POWERON:   return "Normal_on";
      case ESP_RST_EXT:       return "Zunanji reset";
      case ESP_RST_SW:        return "esp_restart()";
      case ESP_RST_PANIC:     return "Crash/Panic";
      case ESP_RST_INT_WDT:   return "Interrupt Watchdog";
      case ESP_RST_TASK_WDT:  return "Task Watchdog";
      case ESP_RST_WDT:       return "RST_WDT";
      case ESP_RST_DEEPSLEEP: return "Deep Sleep";
      case ESP_RST_BROWNOUT:  return "Brownout reset";
      case ESP_RST_SDIO:      return "Reset SDIO";
      default:                return "Nedefiniran razlog";
    }
  }
#endif

// --RSSIQuality-------
int8_t getRSSIasQuality(int8_t RSSI) {
  return constrain(2 * (RSSI + 100), 0, 100);
}

// --PHYmodE------
const char *PHYmodE(uint8_t wifi_get_phy_mode) {
  switch (wifi_get_phy_mode) {
    case 1: return "802.11B";
    case 2: return "802.11G";
    case 3: return "802.11N";
  }
  return "PHY Unknown";
}

//  FlashMode_t ideMode = ESP.getFlashChipMode();
#if defined(ESP32)
const char * FlashChipMode(int ideMode) {
  switch(ideMode) {
    case 0: return "QIO";
    case 1: return "QOUT";
    case 2: return "DIO";
    case 3: return "DOUT";
    default: return "UNKNOWN";
  }
}
#elif defined(ESP8266)
const char * FlashChipMode(FlashMode_t ideMode) {
  switch(ideMode) {
    case FM_QIO: return "QIO";
    case FM_QOUT: return "QOUT";
    case FM_DIO: return "DIO";
    case FM_DOUT: return "DOUT";
    default: return "UNKNOWN";
  }
}
#endif

template <size_t N>
String toCleanString(const std::array<char, N>& arr) {
  return String(arr.data());
}

// --ByteToKbyteToMbytE-------
float formatBytesToFloat(size_t bytes) {
  if (bytes < 1024) {
    return (float)bytes;
  } else if (bytes < (1024 * 1024)) {
    return (float)(bytes / 1024.0);
  } else {
    return (float)(bytes / 1024.0 / 1024.0);
  }
}

using ByteBuffer = std::array<char, 16>;
ByteBuffer formatBytesToString(size_t bytes) {
  ByteBuffer outBuffer;
  
  if (bytes < 1024) {
      snprintf(outBuffer.data(), outBuffer.size(), "%zu B", bytes);
  } else if (bytes < (1024UL * 1024UL)) {
      snprintf(outBuffer.data(), outBuffer.size(), "%.2f KB", bytes / 1024.0);
  } else if (bytes < (1024UL * 1024UL * 1024UL)) {
      snprintf(outBuffer.data(), outBuffer.size(), "%.2f MB", bytes / 1024.0 / 1024.0);
  } else {
      snprintf(outBuffer.data(), outBuffer.size(), "%.2f GB", bytes / 1024.0 / 1024.0 / 1024.0);
  }
  
  return outBuffer;
}

void get_file_list_json(JsonArray& jsonArray) {
  #if defined(ESP32)
    File root = LittleFS.open("/");
    if (!root || !root.isDirectory()) return;

    File file = root.openNextFile();
    while (file) {
      if (!file.isDirectory()) {
        JsonObject fileObj = jsonArray.add<JsonObject>();
        fileObj[F("name")] = file.name();
        fileObj[F("size")] = String(formatBytesToString(file.size()).data());
      }
      file = root.openNextFile();
    }

  #elif defined(ESP8266)
    Dir dir = LittleFS.openDir("/");
    while (dir.next()) {
      JsonObject fileObj = jsonArray.add<JsonObject>(); 
      fileObj[F("name")] = dir.fileName();
      fileObj[F("size")] = String(formatBytesToString(dir.fileSize()).data());
    }
  #endif
}


using UptimeBuffer = std::array<char, 25>;
UptimeBuffer getUptime() {
    UptimeBuffer outBuffer;
    
    unsigned long total_seconds = millis() / 1000UL;
    
    uint16_t days = total_seconds / 86400UL;
    total_seconds %= 86400UL;
    
    uint8_t hours = total_seconds / 3600UL;
    total_seconds %= 3600UL;
    
    uint8_t minutes = total_seconds / 60UL;
    uint8_t seconds = total_seconds % 60UL;
    
    snprintf(outBuffer.data(), outBuffer.size(), "%04d-%02d:%02d:%02d", days, hours, minutes, seconds);
    
    return outBuffer;
}

using PowerBuffer = std::array<char, 16>;
PowerBuffer formatPower(float watts) {
  PowerBuffer outBuffer;
  if (watts < 1000.0f) {
      snprintf(outBuffer.data(), outBuffer.size(), "%.2f W", watts);
  } else if (watts < 1000000.0f) {
      snprintf(outBuffer.data(), outBuffer.size(), "%.2f kW", watts / 1000.0f);
  } else {
      snprintf(outBuffer.data(), outBuffer.size(), "%.2f MW", watts / 1000000.0f);
  }
  return outBuffer;
}

const char* Wifi_status(wl_status_t wifistatus) {
  switch (wifistatus) {
    case WL_NO_SHIELD:       return "No shield";
    case WL_IDLE_STATUS:     return "Idle status";
    case WL_NO_SSID_AVAIL:   return "No SSID avail";
    // case WL_SCAN_COMPLETED: return "Scan completed"; // OSTRANJENO - Ne obstaja v wl_status_t
    case WL_CONNECTED:       return "Povezan";
    case WL_CONNECT_FAILED:  return "Failed";
    case WL_CONNECTION_LOST: return "Connection lost";
    case WL_DISCONNECTED:    return "Disconnected";
    
    #if defined(ESP8266)
    case WL_WRONG_PASSWORD:  return "Wrong password";
    #endif
    
    default:                 break;
  }
  return "WiFi Unknown";
}

void deleteFile(const char* path) {
  if (LittleFS.remove(path)) {
    // DEBUG_PRINTF("%s - file deleted\r\n", path);
  }
}

void createDir(const char* path) {
  #if defined(ESP32)
  if (LittleFS.mkdir(path)) {
    // OK
  }
  #endif
}

void removeDir(const char* path) {
  #if defined(ESP32)
  if (LittleFS.rmdir(path)) {
    // OK
  }
  #endif
}

String getCleanBoardName() {
    String b = ARDUINO_BOARD;
    b.replace("_GENERIC", "");
    b.trim();
    return b;
}
