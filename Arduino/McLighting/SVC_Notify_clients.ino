void notifyClients(const String& topic, const String& msg) {
  #ifdef ENABLE_webSockets
    ws.textAll(msg); 
  #endif

  #ifdef ENABLE_MQTT
    if (MQTT_connected == 1) {
      if (topic != notify_none) {
        publishMqtt(topic.c_str(), msg.c_str());
      }
    }
  #endif

  #ifdef ENABLE_BLINK
    esp_blink_mode(ESP_led, blink_MSG);
  #endif
}
