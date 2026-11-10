void handler_save_palettes();
bool write_strip_state_FS();
bool read_strip_state_FS();

void Trigger_Save_Config();
void Trigger_Save_State();

#ifdef ENABLE_webSockets
  void wsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
#endif