#ifdef ENABLE_MQTT

bool reconnectMqtt = false;
uint32_t lastReconnectMQTT = 0;

static const char sTopicFormat[] PROGMEM = "%s/%s/%s";

#endif 