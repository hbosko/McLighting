// TEST!!! //
// auto update 
// tooo lazyyy to finish...


/* recieve json structure
{
"upd":true,
"deleteFS":false,
"type":"SENSOR", //BoardType
"board":"esp8266",
"file":"sensor_0.5.8.bin",
"version":"0.5.8",
"url":"http://10.11.12.13/files/esp8266/sensor/sensor_0.5.8.bin"
}
*/

#ifdef FIRMWARE_UPDATE

#if defined(ESP8266)
  #include <ESP8266httpUpdate.h>
#elif defined(ESP32)
  #include <HTTPUpdate.h>
#endif

void httpUpdate_init(){
  #if defined(ESP8266)
    ESPhttpUpdate.setClientTimeout(2000);
    DEBUG_PRINTLN("ESPhttpUpdate Started");
  #elif defined(ESP32)
    httpUpdate.setClientTimeout(2000);
    DEBUG_PRINTLN("httpUpdate Started");
  #endif
}

void httpUpdate_loop() {

  if (OTA_fw_update && !OTA_fw_update_running) {
    OTA_fw_update_running = true;
    
    do_fw_upgrade(FW_file_url, fw_deleteFS);
    
    OTA_fw_update = false;
    OTA_fw_update_running = false;
  }

}

void CheckPayload_fw(const char* payload, size_t len, const char* topic) {
  JsonDocument fw_doc;
  
  DeserializationError error = deserializeJson(fw_doc, payload, len);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  bool fw_upgrade = fw_doc[F("upd")] | false;

  if (fw_upgrade == true) {
    const char* fw_version = fw_doc[F("version")] | "";
    const char* fw_type    = fw_doc[F("type")] | "";
    const char* fw_board   = fw_doc[F("board")] | "";
    const char* fw_file    = fw_doc[F("file")] | "";
    bool fw_deleteFS       = fw_doc[F("deleteFS")] | false;
    
    const char* url_cand = fw_doc[F("url")] | "";

    strncpy(FW_file_url, url_cand, sizeof(FW_file_url) - 1);
    FW_file_url[sizeof(FW_file_url) - 1] = '\0';

    DEBUG_PRINT("Prejet FW_file_url: ");
    DEBUG_PRINTLN(FW_file_url);

    int ver_cmp = strcmp(FIRMWARE_VERSION, fw_version);

    if (strcmp(fw_board, FW_BOARD_TYPE_STR) == 0 && ver_cmp < 0) {
      #ifdef ENABLE_MQTT
        notifyClients(mqtt_firmware_status, fw_update(FW_status_START, "0")); 
      #endif
      
      OTA_fw_update = true;
      DEBUG_PRINTLN("FIRMWARE preverjanje: Vse je OK! Začenjam posodobitev...");
    }
    else {
      DEBUG_PRINTLN("FIRMWARE preverjanje: Tip modula ali plošče ne ustreza, ali pa je različica prestara.");
    }

    if (ver_cmp > 0) {
      DEBUG_PRINTLN("Status različice: Prejeta različica je starejša (Downgrade).");
    }
    else if (ver_cmp < 0) {
      DEBUG_PRINTLN("Status različice: Prejeta različica je novejša (Upgrade).");
    }
    else {
      DEBUG_PRINTLN("Status različice: Različici sta popolnoma enaki.");
    }

    if (strcmp(fw_board, FW_BOARD_TYPE_STR) != 0) {
      DEBUG_PRINTLN("Preverjanje plošče: NAPAKA (Napačna plošča)");
    } else {
      DEBUG_PRINTLN("Preverjanje plošče: ODLIČNO (Pravilna plošča)");
    }
  }
}

void do_fw_upgrade(const char* server_file, bool deleteFS) {
  DEBUG_PRINT("Zagon posodobitve iz datoteke: ");
  DEBUG_PRINTLN(server_file);
  DEBUG_PRINT("Brisanje datotečnega sistema (deleteFS): ");
  DEBUG_PRINTLN(deleteFS);

  WiFiClient client;
  t_httpUpdate_return ret = HTTP_UPDATE_NO_UPDATES;

  #if defined(ESP8266)
    ESPhttpUpdate.rebootOnUpdate(false);
    ESPhttpUpdate.onStart(update_started);
    ESPhttpUpdate.onEnd(update_finished);
    ESPhttpUpdate.onProgress(update_progress);
    ESPhttpUpdate.onError(update_error);

    if (deleteFS == true) {
      DEBUG_PRINTLN("Posodabljam FileSystem (LittleFS/SPIFFS)...");
      ret = ESPhttpUpdate.updateFS(client, server_file);
    } else {
      DEBUG_PRINTLN("Posodabljam Firmware binarno datoteko...");
      ret = ESPhttpUpdate.update(client, server_file);
    }

  #elif defined(ESP32)
    httpUpdate.rebootOnUpdate(false);
    httpUpdate.onStart(update_started);
    httpUpdate.onEnd(update_finished);
    httpUpdate.onProgress(update_progress);
    httpUpdate.onError(update_error);

    if (deleteFS == true) {
      DEBUG_PRINTLN("Posodabljam ESP32 FileSystem...");
      ret = httpUpdate.updateFs(client, server_file);
    } else {
      DEBUG_PRINTLN("Posodabljam ESP32 Firmware...");
      ret = httpUpdate.update(client, server_file);
    }
  #endif

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      #if defined(ESP8266)
        Serial.printf("HTTP_UPDATE_FAILED Koda napake (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        update_failed(ESPhttpUpdate.getLastErrorString().c_str());
      #elif defined(ESP32)
        Serial.printf("HTTP_UPDATE_FAILED Koda napake (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
        update_failed(httpUpdate.getLastErrorString().c_str());
      #endif
      break;

    case HTTP_UPDATE_NO_UPDATES:
      DEBUG_PRINTLN("HTTP_UPDATE_NO_UPDATES: Ni na voljo novejših posodobitev na strežniku.");
      break;

    case HTTP_UPDATE_OK:
      DEBUG_PRINTLN("HTTP_UPDATE_OK: Posodobitev uspešno zaključena!");
      delay(2000);
      esp_reboot = true;
      break;
  }
}

#endif

