#ifdef ENABLE_MQTT

/*
void MQTT_LOOP(){
  if (WiFi.status() == WL_CONNECTED && MQTT_connected == 0) {
    uint32_t currentMillisMQTT = millis();
    if (currentMillisMQTT - lastReconnectMQTT > 10000UL) { // Poskusi vsakih 10 sekund
      lastReconnectMQTT = currentMillisMQTT;
      DEBUG_PRINTLN("=> MQTT_LOOP: Poskušam obnoviti asinhrono povezavo z MQTT brokerjem...");
      connectToMqtt();
    }
  }
}
*/

void publishMqtt(const char *topic, const char* msg, size_t len = 0) {
  const char* mtype = MODULE_TYPE(conf_type);

  if (MQTT_connected == 1) {
    char pubuf[128];
    snprintf_P(pubuf, sizeof(pubuf), sTopicFormat, mtype, device_name, topic);
    size_t payloadLen = (len > 0) ? len : strlen(msg);
    mqttClient.publish(pubuf, 0, false, msg, payloadLen);
    DEBUG_PRINTF("Publish topic: %s | PAYLOAD: %s\r\n", pubuf, msg);
  }
}

void subscribeMqtt(const char *topic) {
  const char* mtype = MODULE_TYPE(conf_type);

  if (MQTT_connected == 1){
    char subuf[128];
    snprintf_P(subuf, sizeof(subuf), sTopicFormat, mtype, device_name, topic);
    mqttClient.subscribe(subuf, 2);
    DEBUG_PRINTF("subscribe Mqtt: %s\r\n", subuf);
  }
}


void init_MQTT_async(){
  #if defined(ESP32)
    WiFi.onEvent(WiFiEvent);
  #endif

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.setCredentials(WMconfig.WMmqttUser.c_str(), WMconfig.WMmqttPass.c_str());
  mqttClient.setServer(WMconfig.WMmqttServer.c_str(), WMconfig.WMmqttPort);
}

void connectToMqtt() {
  DEBUG_PRINTLN("=> Povezovanje na MQTT...");
  mqttClient.connect();
}

#if defined(ESP32)
void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch(event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      DEBUG_PRINTLN("WiFi connected and IP obtained.");
      connectToMqtt();
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      DEBUG_PRINTLN("WiFi lost connection");
      MQTT_connected = 0;
      MQTT_client = "Disconnected";
      //mqttReconnectTimer.detach(); 
        break;
      default:
        break;
    }
}
#endif

void onMqttConnect(bool sessionPresent) {
  MQTT_connected = 1;
  MQTT_client = "Connected";
  //mqttReconnectTimer.detach();

  DEBUG_PRINT("mqqtIP address: ");
  DEBUG_PRINTLN(WiFi.localIP());
  DEBUG_PRINTLN("Connected to MQTT.");
  
  mqttClient.subscribe(mqtt_ping, 2);

  #ifdef FIRMWARE_UPDATE
    char subuf[128];
    snprintf_P(subuf, sizeof(subuf), sTopicFormat, device_name, "firmware");
    mqttClient.subscribe(subuf, 2);
  #endif

  notifyClients(mqtt_announce, ESPinfo());
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  DEBUG_PRINTF("Disconnected from MQTT. Reason: %d\n", (int)reason);
  MQTT_connected = 0;
  MQTT_client = "Disconnected";

  if (WiFi.status() == WL_CONNECTED) {
    //mqttReconnectTimer.once(10, connectToMqtt);
  }
}

void onMqttPublish(uint16_t packetId) {
  DEBUG_PRINT("Publish acknowledged. PacketId: ");
  DEBUG_PRINTLN(packetId);
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  DEBUG_PRINTF("Subscribe acknowledged. PacketId: %d, QoS: %d\r\n", packetId, qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  DEBUG_PRINTF("Unsubscribe acknowledged. PacketId: %d\r\n", packetId);
}

void onMqttMessage(const char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {

  DEBUG_PRINTF("--- MQTT Message ---\r\nTopic: %s\r\nMsg: %.*s\r\n--------\r\n", topic, (int)len, payload);

  #ifdef FIRMWARE_UPDATE
    if (strstr(topic, "firmware") != NULL) {
      DEBUG_PRINTLN("topic is: firmware ");
      CheckPayload_fw(payload, len, topic);
    }
  #endif

  if (len == 4 && strncmp(payload, "ping", 4) == 0) {
    notifyClients(mqtt_announce, ESPinfo());
  }

  if (strstr(topic, "reboot") != NULL) {
    if (len == 3 && strncmp(payload, "rbt", 3) == 0) {
      esp_reboot = true;
    }
  }
}


#endif