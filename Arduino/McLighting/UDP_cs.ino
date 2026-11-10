
#ifdef ENABLE_UDP

  //todo: autoscan if master or slave.. with same "BoardType"!, now works only if you scan from website...

  void udp_cs_init() {
    udp.begin(udpPort);
    UDP_connected = 1;
    DEBUG_PRINTF("UDP port: %d\r\n", udpPort);
  }


  void UDP_mode_json() {
    if (WS2812FXStripSettings.udpMaster) {
      Udp_mode_str = "master";
    } else if (WS2812FXStripSettings.udpReciever) {
      Udp_mode_str = "reciever";
    } else {
      Udp_mode_str = "standalone";
    }
  }

  void addDeviceToList(const char* ip, const char* typeData) {

    for (int i = 0; i < discoveredDeviceCount; i++) {
      if (strcmp(discoveredDevices[i].ip, ip) == 0) {
        discoveredDevices[i].lastSeen = millis();  
        strncpy(discoveredDevices[i].type, typeData, 63);
        discoveredDevices[i].type[63] = '\0';
        return;
      }
    }

    if (discoveredDeviceCount >= MAX_DISCOVERED_DEVICES) return;

    strncpy(discoveredDevices[discoveredDeviceCount].ip, ip, 15);
    discoveredDevices[discoveredDeviceCount].ip[15] = '\0';
    
    strncpy(discoveredDevices[discoveredDeviceCount].type, typeData, 63);
    discoveredDevices[discoveredDeviceCount].type[63] = '\0';

    discoveredDevices[discoveredDeviceCount].lastSeen = millis();
    discoveredDeviceCount++;
  }


  void discoverSameModules() {
    char pingMsg[64];
    snprintf(pingMsg, sizeof(pingMsg), "PING|%s", MODULE_TYPE(conf_type));

      DEBUG_PRINT("discover Same Modules pingMsg: ");
      DEBUG_PRINTLN(pingMsg);

    udp.beginPacket(IPAddress(255, 255, 255, 255), udpPort);
    udp.write((const uint8_t*)pingMsg, strlen(pingMsg));
    udp.endPacket();
  }


  void handleUdpSync_loop() {

    int packetSize = udp.parsePacket();
    if (packetSize > 0) {
      char udpBuffer[128];
      if (packetSize > 127) packetSize = 127;
      udp.read(udpBuffer, packetSize);
      udpBuffer[packetSize] = '\0';

        DEBUG_PRINT("udpBuffer: ");
        DEBUG_PRINTLN(udpBuffer);

      if (strncmp(udpBuffer, "PING|", 5) == 0) {
        char* searchedType = udpBuffer + 5;

            DEBUG_PRINT("pongMsg searched Type: ");
            DEBUG_PRINTLN(searchedType);
            
        if (strcmp(searchedType, MODULE_TYPE(conf_type)) == 0) {
          char pongMsg[96];
          
          const char* myMode = WS2812FXStripSettings.udpMaster ? "master" : (WS2812FXStripSettings.udpReciever ? "slave" : "standalone");

          snprintf(pongMsg, sizeof(pongMsg), "PONG|%s|%s|%s", MODULE_TYPE(conf_type), MODULE_sub_TYPE(conf_sub_type), myMode);

          DEBUG_PRINT("Send pongMsg: ");
          DEBUG_PRINTLN(pongMsg);
          
          udp.beginPacket(udp.remoteIP(), udpPort); 
          udp.write((const uint8_t*)pongMsg, strlen(pongMsg));
          udp.endPacket();
        }
        return;

      }

      if (strncmp(udpBuffer, "PONG|", 5) == 0) {
        String ipStr = udp.remoteIP().toString();
        const char* receivedData = udpBuffer + 5;
        
        addDeviceToList(ipStr.c_str(), receivedData); 
        return; 
      }

      if (!WS2812FXStripSettings.udpReciever) return;

      if (millis() - lastUdpSendTime < 5) return;

      char* pipeChar = strchr(udpBuffer, '|');
      if (pipeChar != nullptr) {
        *pipeChar = '\0';  
        char* receivedSubType = udpBuffer;
        char* actualPayload = pipeChar + 1;

        if (strcmp(receivedSubType, MODULE_sub_TYPE(conf_sub_type)) == 0) {
          bool backupMaster = WS2812FXStripSettings.udpMaster;
          WS2812FXStripSettings.udpMaster = false; 

          CheckPayload(actualPayload, strlen(actualPayload));
            DEBUG_PRINT("actual Payload: ");
            DEBUG_PRINTLN(actualPayload);

          WS2812FXStripSettings.udpMaster = backupMaster;
        }
      }
    }
  }


  void sendUdpUnicastToGroup(const char* payload, size_t len) {
    if (!WS2812FXStripSettings.udpMaster) return;

    DEBUG_PRINT("send Udp Unicast To Group: ");
    DEBUG_PRINTLN(payload);

    char syncPayload[128];
    snprintf(syncPayload, sizeof(syncPayload), "%s|%s", MODULE_sub_TYPE(conf_sub_type), payload);

    int sentCount = 0;
    const char* localSubType = MODULE_sub_TYPE(conf_sub_type);
    const char* localType = MODULE_TYPE(conf_type);

    char targetMatchString[64];
    snprintf(targetMatchString, sizeof(targetMatchString), "%s|%s", localType, localSubType);
    size_t matchLen = strlen(targetMatchString);

    for (int i = 0; i < discoveredDeviceCount; i++) {
      if (strncmp(discoveredDevices[i].type, targetMatchString, matchLen) == 0 && 
        (discoveredDevices[i].type[matchLen] == '\0' || discoveredDevices[i].type[matchLen] == '|')) {
        
        IPAddress targetIP;
        if (WiFi.hostByName(discoveredDevices[i].ip, targetIP) == 1) {

          
          udp.beginPacket(targetIP, udpPort);
          udp.write((const uint8_t*)syncPayload, strlen(syncPayload));
          udp.endPacket();
          
          sentCount++;
          #ifdef DEBUG
            DEBUG_PRINTF("UDP Unicast send to %s\r\n", discoveredDevices[i].ip);
          #endif
        }
      }
    }

    #ifdef DEBUG
      DEBUG_PRINTF("Sync finished. Packet send to %d board.\r\n", sentCount);
    #endif

    lastUdpSendTime = millis();
  }


  void serverOn_UDP() {
    server.on(serverURL_get_HTML, HTTP_GET, [&](AsyncWebServerRequest * request) {
      if (request->client() != nullptr) {
        AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", udp_html_gz, udp_html_gz_len);
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
      }
    });

    //  server.on(serverURL_get_UDP, HTTP_GET, [](AsyncWebServerRequest * request) {
    server.on("/udp/get", HTTP_GET, [](AsyncWebServerRequest* request) {
      const char* modeString = "standalone";
      if (WS2812FXStripSettings.udpMaster) {
        modeString = "master";
      } else if (WS2812FXStripSettings.udpReciever) {
        modeString = "slave";
      }

      char buf[64];
      snprintf(buf, sizeof(buf), "{\"mode\":\"%s\"}", modeString);
      request->send(200, "application/json", buf);
    });


    //server.on(serverURL_set_UDP, HTTP_ANY, [](AsyncWebServerRequest* request) {
    server.on("/udp/set", HTTP_ANY, [](AsyncWebServerRequest* request) {
      if (request->hasParam("master", true) || request->hasParam("master", false)) {
        AsyncWebParameter* p = request->getParam("master", request->hasParam("master", true));
        String val = p->value();
        WS2812FXStripSettings.udpMaster = (val == "1" || val == "true");
      }

      if (request->hasParam("slave", true) || request->hasParam("slave", false)) {
        AsyncWebParameter* p = request->getParam("slave", request->hasParam("slave", true));
        String val = p->value();
        WS2812FXStripSettings.udpReciever = (val == "1" || val == "true");
      }

      #ifdef DEBUG
        DEBUG_PRINTF("UDP Config Saved -> Master: %d, Slave: %d\r\n",
        WS2812FXStripSettings.udpMaster, WS2812FXStripSettings.udpReciever);
      #endif

        UDP_mode_json();

      save_strip_config_ticker.detach();
      save_strip_config_ticker.once(3, Trigger_Save_Config);

      request->send(200, "text/plain", "OK");
    });

    //server.on(serverURL_UDP_discover, HTTP_GET, [](AsyncWebServerRequest* request) {
    server.on("/udp/discover", HTTP_GET, [](AsyncWebServerRequest* request) {
      discoveredDeviceCount = 0;
      discoverSameModules();
      request->send(200, "text/plain", "Searching started...");
    });

    //server.on(serverURL_UDP_list, HTTP_GET, [](AsyncWebServerRequest* request) {
    server.on("/udp/list", HTTP_GET, [](AsyncWebServerRequest* request) {
      JsonDocument doc;
      JsonArray arr = doc.to<JsonArray>();

      for (int i = 0; i < discoveredDeviceCount; i++) {
        JsonObject obj = arr.add<JsonObject>();
        obj["ip"] = discoveredDevices[i].ip;
        
        String fullType = String(discoveredDevices[i].type);
        int lastPipe = fullType.lastIndexOf('|');
        
        if (lastPipe != -1 && (fullType.endsWith("master") || fullType.endsWith("slave") || fullType.endsWith("standalone"))) {
          obj["type"] = fullType.substring(0, lastPipe);
          obj["mode"] = fullType.substring(lastPipe + 1);
        } else {
          obj["type"] = fullType;
          obj["mode"] = "unknown";
        }
      }

      AsyncResponseStream* response = request->beginResponseStream("application/json");
      if (discoveredDeviceCount == 0) {
        response->print("[]");
      } else {
        serializeJson(doc, *response);
      }
      request->send(response);
    });


  }

#endif