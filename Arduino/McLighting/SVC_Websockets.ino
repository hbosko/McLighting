#ifdef ENABLE_webSockets

void init_webSockets() {
    WS_enabled = 1;
  ws.onEvent(wsEvent);
  server.addHandler(&ws);
  DEBUG_PRINTLN(F("WebSockets server OK."));
}

void wsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
  
  if (client == nullptr) return;

  if (type == WS_EVT_CONNECT) {
    WS_client = client->remoteIP().toString();
    
    DEBUG_PRINTF("WS conected IP: %s\r\n", WS_client.c_str());
    notifyClients(notify_evt, WS_client.c_str());
  } 
  
  else if (type == WS_EVT_DISCONNECT) {
    DEBUG_PRINT(WS_client);
    DEBUG_PRINTLN(F(" - WS disconected."));
    
    //WS_client = "Disconnected";
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
    WS_client = "Error";
  } 
  
  else if (type == WS_EVT_PONG) {
    DEBUG_PRINTLN(F("WS pong (ping)."));
  }
}

#endif
