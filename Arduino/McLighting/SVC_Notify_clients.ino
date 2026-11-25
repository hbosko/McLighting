void notifyClients(const char* topic, const char* msg) {
  //websocket disabled in AP mode, bufferOwerflov, OOM (esp8266)
  if (apActive) {
    return;
  }

  #ifdef ENABLE_webSockets
    if (ws.count() > 0) {
      ws.textAll(msg);
    }
  #endif

  #ifdef ENABLE_MQTT
    if (MQTT_connected == 1) {
      if (topic != nullptr && strcmp(topic, notify_none) != 0) {
        publishMqtt(topic, msg);
      }
    }
  #endif

  #ifdef ENABLE_BLINK
    esp_blink_mode(ESP_led, blink_MSG);
  #endif
}