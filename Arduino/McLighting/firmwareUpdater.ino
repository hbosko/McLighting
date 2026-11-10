// https://github.com/IPdotSetAF/ESPAsyncHTTPUpdateServer

#define FW_status_START 0
#define FW_status_FINISH 1
#define FW_status_ERROR 2
#define FW_status_FAILED 3
#define FW_status_PROGRESS 4


void init_firmwareUpdater(){

  updateServer.setup(&server, serverURL_ESP_fw);
/*
  updateServer.setup(&server, serverURL_ESP_fw, FW_username, FW_password);
*/
  updateServer.onUpdateBegin = [](const UpdateType type, int &result) {
    DEBUG_PRINTLN("Update started : " + String(type));
      update_started();
      //you can force abort the update like this if you need to:
      //result = UpdateResult::UPDATE_ABORT;        
  };

  Update.onProgress([](size_t progress, size_t total) {
    update_progress(progress, total);
  });

  updateServer.onUpdateEnd = [](const UpdateType type, int &result) {
    DEBUG_PRINTLN("Update finished : " + String(type) + " result: " + String(result));
    
    if (result == 0) {
      update_finished();
    } else {
      update_error(result);
    }
  };
}


String fw_update(uint8_t state, const char* msg){
  JsonDocument doc;
    JsonObject fw_Data_info = doc["info"].to<JsonObject>();

  if (state == FW_status_FINISH){
    fw_Data_info[F("st")] = F("Finish");
  }
  else if (state == FW_status_START){
    fw_Data_info[F("upgr")] = F("Start");
  }
  else if (state == FW_status_ERROR){
    fw_Data_info[F("st")] = F("Error");
    fw_Data_info[F("msg")] = msg;
  }
  else if (state == FW_status_FAILED){
    fw_Data_info[F("st")] = F("Failed");
    fw_Data_info[F("msg")] = msg;
  }
  else if (state == FW_status_PROGRESS){
    fw_Data_info[F("st")] = F("Progress");
    fw_Data_info[F("msg")] = msg;
  }

  fw_Data_info[F("name")]  = device_name;
  fw_Data_info[F("ip")]    = WiFi.localIP().toString();
  fw_Data_info[F("brd")]   = F(ARDUINO_BOARD);
  fw_Data_info[F("ver")]   = FIRMWARE_VERSION;
  
  #ifdef ENABLE_MQTT_
    fw_Data_info[F("modul")] = MODULE_TYPE;
    fw_Data_info[F("sub_modul")]    = MODULE_TYPE;
  #endif

  String json = "";
  serializeJson(doc, json);
  return json;
}



void update_started() {
  DEBUG_PRINTLN("FW CALLBACK: HTTP update process started");
  notifyClients(notify_evt,"Firmware update starting");

  #ifdef ENABLE_MQTT
    //publishMqtt(mqtt_firmware_status, fw_update(FW_status_START, "0").c_str());
    notifyClients(mqtt_firmware_status, fw_update(FW_status_START, "0"));
  #endif

    OTA_fw_update_running = true;
}

void update_finished() {
  DEBUG_PRINTLN("FW CALLBACK: HTTP update process finished");
    notifyClients(notify_evt,"Firmware update done!");

  #ifdef ENABLE_MQTT
    //publishMqtt(mqtt_firmware_status, fw_update(FW_status_FINISH, "0").c_str());
    notifyClients(mqtt_firmware_status, fw_update(FW_status_FINISH, "0"));
  #endif

    OTA_fw_update_running = true;
}

void update_progress(int cur, int total) {
  int percent = (total > 0) ? (cur * 100) / total : 0;

  char sendData[64];
  snprintf(sendData, sizeof(sendData), "%d%% (%d/%d B)", percent, cur, total);
      
  Serial.printf("FW CALLBACK: HTTP update process at %s...\n", sendData);
  
  #ifdef ENABLE_MQTT
    static int last_reported_percent = -1;
    // MQTT obvestilo vsakih 10%
    if (percent % 10 == 0 && percent != last_reported_percent) { 
      last_reported_percent = percent;
      //publishMqtt(mqtt_firmware_status, fw_update(FW_status_PROGRESS, sendData).c_str());
      notifyClients(mqtt_firmware_status, fw_update(FW_status_PROGRESS, sendData));
    }
  #endif
  
  OTA_fw_update_running = true;
}

void update_error(int err) {
  char sendData[16];
  snprintf(sendData, sizeof(sendData), "%d", err); 
  
  Serial.printf("FW CALLBACK: HTTP update fatal error code %d\n", err);
  #ifdef ENABLE_MQTT
    //publishMqtt(mqtt_firmware_status, fw_update(FW_status_ERROR, sendData).c_str());
    notifyClients(mqtt_firmware_status, fw_update(FW_status_ERROR, sendData));
  #endif
  OTA_fw_update_running = false;
}

void update_failed(const char* err) {
  DEBUG_PRINT("FW update_failed Error: ");
  DEBUG_PRINTLN(err);
  #ifdef ENABLE_MQTT
    //publishMqtt(mqtt_firmware_status, fw_update(FW_status_FAILED, err).c_str());
    notifyClients(mqtt_firmware_status, fw_update(FW_status_FAILED, err));
  #endif
  OTA_fw_update_running = false;
}