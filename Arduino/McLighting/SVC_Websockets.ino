#ifdef ENABLE_webSockets

void init_webSockets() {
    WS_enabled = 1;
  ws.onEvent(wsEvent);
  server.addHandler(&ws);
  DEBUG_PRINTLN(F("WebSockets server OK."));
}

void wsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  
 // if (apActive) { return;  } // disable websocket if in AP mode! BufferOverFlow... OOM!

  if (client == nullptr) return;

  if (type == WS_EVT_CONNECT) {
      snprintf(WS_client, sizeof(WS_client), "%s", client->remoteIP().toString().c_str());
    
    DEBUG_PRINTF("WS conected IP: %s\r\n", WS_client);
    notifyClients(notify_evt, WS_client);
  } 
  
  else if (type == WS_EVT_DISCONNECT) {
    DEBUG_PRINT(WS_client);
    DEBUG_PRINTLN(F(" - WS disconected."));
    
    snprintf(WS_client, sizeof(WS_client), "Disconnected");
  } 
  
  else if (type == WS_EVT_DATA) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
      
      data[len] = '\0'; 
      
      #ifdef DEBUG
        DEBUG_PRINTF("WS msg recieved: %s\r\n", (char*)data);
      #endif
      
      CheckPayload((char*)data, len);
    }
  }
  
  else if (type == WS_EVT_ERROR) {
    DEBUG_PRINTLN(F("WS err."));
    snprintf(WS_client, sizeof(WS_client), "Error");
  } 
  
  else if (type == WS_EVT_PONG) {
    DEBUG_PRINTLN(F("WS pong (ping)."));
  }
}

#endif
