
void startSoftAccessPoint(const char *ssid, const char *password, const IPAddress &localIP, const IPAddress &gatewayIP) {
  initAP_path();

  #if defined(ESP8266)
    WiFi.persistent(false);
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
    wifi_station_disconnect();
    wifi_station_set_reconnect_policy(false);
    WiFi.disconnect(true);
    delay(50);
    WiFi.mode(WIFI_AP_STA);
    WiFi.setOutputPower(12.0);
  #elif defined(ESP32)
    WiFi.setSleep(false);
    WiFi.disconnect();
    WiFi.mode(WIFI_AP_STA);
  #endif

  apActive = true;
  AP_count = 0;
  DEBUG_PRINTLN(" -- initAP -- ");

  WiFi.softAPConfig(localIP, gatewayIP, subnetMask);
  WiFi.softAP(ssid, password, WIFI_CHANNEL, 0, MAX_CLIENTS);

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", WiFi.softAPIP());
}


void initAP_path() {

  server.on("/connecttest.txt", HTTP_ANY, [](AsyncWebServerRequest *request) {request->redirect("http://logout.net");});
  server.on("/wpad.dat", HTTP_ANY, [](AsyncWebServerRequest *request) {request->send(404);});
  server.on("/ncsi.txt", HTTP_ANY, [](AsyncWebServerRequest *request) {request->send(200, "text/plain", "Microsoft NCSI");});

  server.on("/scan-async", HTTP_GET, [](AsyncWebServerRequest *request) {
    WiFi.scanNetworks(true); 
    request->send(200, "text/plain", F("OK"));
  });

    server.on("/get-networks", HTTP_GET, [](AsyncWebServerRequest *request) {
    int n = WiFi.scanComplete();

    if (n == -1) {
      request->send(200, "text/html", F("<div>⏳ Refresh.ing... <br><br><a href='#' class='scan-btn' onclick='osveziSeznam(); return false;'>SCAN WiF</a></div>"));
    } else if (n == -2 || n == 0) {
      request->send(200, "text/html", F("<div>❌ No WifI in range. <br><br><a href='#' class='scan-btn' onclick='sproziInOsvezi(); return false;'>reTRY</a></div>"));
    } else {
      
      AsyncResponseStream *response = request->beginResponseStream("text/html");

      int indices[n];
      for (int i = 0; i < n; i++) indices[i] = i;

      //  sortiranje
      for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
          if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
            std::swap(indices[i], indices[j]);
          }
        }
      }

      // Filtriranje duplikatov
      for (int i = 0; i < n; i++) {
        if (indices[i] == -1) continue;
        String cssid = WiFi.SSID(indices[i]);
        for (int j = i + 1; j < n; j++) {
          if (cssid == WiFi.SSID(indices[j])) {
            indices[j] = -1;
          }
        }
      }

      char qualBuf[8];

      for (int i = 0; i < n; i++) {
        if (indices[i] == -1) continue;
        if (WiFi.SSID(indices[i]).length() == 0) continue;

        int32_t rssi = WiFi.RSSI(indices[i]);
        int quality = (rssi <= -100) ? 0 : (rssi >= -50) ? 100 : 2 * (rssi + 100);
        
        snprintf(qualBuf, sizeof(qualBuf), "%d", quality);

        response->print(F("<a href='#' class='link' onclick='c(this); return false;'>📶 <span>"));
        response->print(WiFi.SSID(indices[i]));
        response->print(F("</span> ("));
        response->print(qualBuf);
        response->print(F("%)</a>\n"));
      }

      request->send(response);
      WiFi.scanDelete();
    }
  });


  server.on("/generate_204", HTTP_ANY, [](AsyncWebServerRequest *request) {request->redirect(localIPURL);});
  server.on("/gen_204", HTTP_ANY, [](AsyncWebServerRequest *request) {request->redirect(localIPURL);});
  server.on("/redirect", HTTP_ANY, [](AsyncWebServerRequest *request) {request->redirect(localIPURL);});
  server.on("/hotspot-detect.html", HTTP_ANY, [](AsyncWebServerRequest *request) {request->redirect(localIPURL);});
  server.on("/canonical.html", HTTP_ANY, [](AsyncWebServerRequest *request) {request->redirect(localIPURL);});
  server.on("/success.txt", HTTP_ANY, [](AsyncWebServerRequest *request) {request->send(200, "text/plain", "success");});
  server.on("/favicon.ico", HTTP_ANY, [](AsyncWebServerRequest *request) {request->send(404);});
  server.on("/success.txt", HTTP_ANY, [](AsyncWebServerRequest *request) {request->send(200, "text/plain", F("success"));});
  server.on("/library/test/success.html", HTTP_ANY, [](AsyncWebServerRequest *request) {request->send(200, "text/html", F("<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>"));});
 
 
  /*
    server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request) {
      AsyncWebServerResponse *response = request->beginResponse(200, "text/html", SendHTML());
      response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
      response->addHeader("Pragma", "no-cache");
      response->addHeader("Expires", "-1");
      request->send(response);
    });
  */

  server.onNotFound([](AsyncWebServerRequest *request) {
    const String host = request->host();
    
    if (strstr(host.c_str(), "4.4.4.1") == nullptr) {
      request->redirect(localIPURL);
      
      DEBUG_PRINT("Captive Portal preusmeril neznan host: ");
      DEBUG_PRINTLN(host.c_str());
    } else {
      request->send(404);
    }
  });
}


void connectToWifi() {
  WiFi.mode(WIFI_STA);

  #if defined(ESP32)
    // WiFi.setTxPower(WIFI_POWER_8_5dBm);
  #endif

  WiFi.setAutoReconnect(false);
  WiFi.disconnect();
  delay(100);

  DEBUG_PRINTLN("Connecting to Wi-Fi...");
  WiFi.begin(WMconfig.WMssid, WMconfig.WMpass);

  wifiMillis = millis();
  wifi_connecting = true;
  WIFI_connected = 0;
}

#if defined(ESP8266)
  void onWifiConnect(const WiFiEventStationModeGotIP &event) {
    (void)event;
    AP_count = 0;
    apActive = false;
    WIFI_connected = 1;
    DEBUG_PRINTLN("Connected to Wi-Fi (ESP8266 Got IP).");
  }

  void onWifiDisconnect(const WiFiEventStationModeDisconnected &event) {
    (void)event;
    WIFI_connected = 0;
    DEBUG_PRINTLN("Disconnected from Wi-Fi (ESP8266).");
    DEBUG_PRINT("IP address: ");
    DEBUG_PRINTLN(WiFi.localIP());
  }

#elif defined(ESP32)
  void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
    WIFI_connected = 1;
    AP_count = 0;
    apActive = false;
    DEBUG_PRINTLN("WiFi connected (ESP32 Got IP)!");
    DEBUG_PRINT("IP address: ");
    DEBUG_PRINTLN(WiFi.localIP());
  }

  void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
    WIFI_connected = 0;
    DEBUG_PRINT("Disconnected from WiFi. Reason: ");
    DEBUG_PRINTLN(info.wifi_sta_disconnected.reason);
  }
#endif


// SETUP
void init_CUSTOM_WIFI() {

  #if defined(ESP8266)
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
  #elif defined(ESP32)
    WiFi.setSleep(false);
  #endif

  DEBUG_PRINTLN(ARDUINO_BOARD);

  if (!LittleFS.exists(wifiPath)) {

    DEBUG_PRINTLN("WiFi Config File NOT found!");
    apActive = true;
    firstBoot = true;

    startSoftAccessPoint(softAPname, softAP_PASS, localIP, gatewayIP);

  } else {
    DEBUG_PRINTLN("connecting to SSID: ");

    dnsServer.stop();
    WiFi.softAPdisconnect(true);
    WiFi.disconnect(true);
    delay(5);
    WiFi.mode(WIFI_STA);

    apActive = false;
    firstBoot = false;

    read_Wifi_from_FS();

    #if defined(ESP8266)
      WiFi.setAutoConnect(false);
      WiFi.setAutoReconnect(true);
      wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
      wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
      WiFi.hostname(device_name);

    #elif defined(ESP32)
      //  WiFi.onEvent(WiFiStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);   // LEAVE COMMENTED!
      WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
      //  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);   // LEAVE COMMENTED!
      WiFi.setHostname(device_name);
    #endif

    #ifdef ENABLE_OTA
      init_OTA(device_name);
    #endif

    #ifdef ENABLE_MDNS
      init_MDNS(device_name);
    #endif

    connectToWifi();
  }
}

// LOOPP
void loop_CUSTOM_WIFI() {
  dnsServer.processNextRequest();

  if (wifi_connecting) {
    if (WiFi.status() == WL_CONNECTED) {
      WiFi.setAutoReconnect(true);
      AP_count = 0;
      apActive = false;
      wifi_connecting = false;
      WIFI_connected = 1;
      
      dnsServer.stop();
      WiFi.softAPdisconnect(true);
      WiFi.mode(WIFI_STA);
      
      DEBUG_PRINTLN("Wi-Fi successfully connected!");
    }
    else if (millis() - wifiMillis >= 15000) { 
      DEBUG_PRINTLN("Timeout! Wifi conn failed.");
      AP_count++;
      wifi_connecting = false;
      WIFI_connected = 0;

      DEBUG_PRINT("AP Count: ");
      DEBUG_PRINTLN(AP_count);

      if (AP_count >= 3) {
        apActive = true;
        DEBUG_PRINTLN("SoftAP warmup...");
        startSoftAccessPoint(softAPname, softAP_PASS, localIP, gatewayIP);
      }
    }
  }

  if (millis() - WM_loop > interval_5s) {
    WM_loop = millis();

    if (apActive == false && WiFi.status() != WL_CONNECTED && wifi_connecting == false) {
      DEBUG_PRINTLN("Wifi lost, again conn...");
      connectToWifi();
    }
    else if (apActive == false && WiFi.status() == WL_CONNECTED) { 
      WIFI_connected = 1; 
    }

    if (apActive == true) {
      #if defined(ESP8266)
        if (WiFi.status() == WL_DISCONNECTED) { wifi_station_disconnect(); }
      #endif

        if (lastTryInAp == 0) { lastTryInAp = millis(); }
        if (wifi_connecting == false && (millis() - lastTryInAp >= RECONNECT_INTERVAL)) {
          lastTryInAp = millis();
          DEBUG_PRINTLN("\n[WifI bg] Try to connect to AP/Router...");
          WiFi.begin(WMconfig.WMssid, WMconfig.WMpass);
          wifiMillis = millis();
          wifi_connecting = true; 
        }
      
    }

    #ifdef ENABLE_MQTT
      if (!apActive && WiFi.status() == WL_CONNECTED && !MQTT_connected) {
        connectToMqtt();
      }
    #endif
  }
}

void handle_wm_cmd(AsyncWebServerRequest *request) {
  int params = request->params();

  const char* odzivStatus = "NAPACNO GESLO";

  for (int i = 0; i < params; i++) {
    const AsyncWebParameter *p = request->getParam(i);
    DEBUG_PRINTF("Param name: %s | value: %s\r\n", p->name().c_str(), p->value().c_str());

    const char* pName = p->name().c_str();
    const char* pValue = p->value().c_str();

    if (strcmp(pName, "rbt") == 0) {
      if (strcmp(pValue, "1") == 0) { 
        notifyClients(notify_evt, notify_OK);
        odzivStatus = "OK - Rebooting...";
        esp_reboot = true;
      } else {
        notifyClients(notify_evt, notify_FAILED);
      }
    }

    if (strcmp(pName, "wm") == 0) {
      if (strcmp(pValue, "1") == 0) { 
        notifyClients(notify_evt, "Reboot and switch to AP mode");
        odzivStatus = "OK - Switching to AP Mode... and RebooT";
        deleteFile(wifiPath);
        esp_reboot = true;
      } else {
        notifyClients(notify_evt, notify_FAILED);
      }
    }
  }

  AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", odzivStatus);
  response->addHeader("Access-Control-Allow-Origin", "*");
  request->send(response);
}


void handle_wifisave(AsyncWebServerRequest *request) {
  int params = request->params();
  bool wifiSpremenjen = false;

  for (int i = 0; i < params; i++) {
    const AsyncWebParameter *p = request->getParam(i);
    DEBUG_PRINTF("Param name: %s | value: %s\r\n", p->name().c_str(), p->value().c_str());

    if (p->isPost() || request->method() == HTTP_POST) {
      const char* pName = p->name().c_str();
      const char* pValue = p->value().c_str();

      if (strcmp(pName, hostNameParam) == 0) {
        strncpy(device_name, pValue, sizeof(device_name) - 1);
        device_name[sizeof(device_name) - 1] = '\0';
        WMconfig.WMhostName = device_name;
        wifiSpremenjen = true;
      }

      if (strcmp(pName, ssidParam) == 0) {
        snprintf(WMconfig.WMssid, sizeof(WMconfig.WMssid), "%s", pValue);
        wifiSpremenjen = true;
      }

      if (strcmp(pName, passParam) == 0) {
        snprintf(WMconfig.WMpass, sizeof(WMconfig.WMpass), "%s", pValue);
        wifiSpremenjen = true;
      }

      #ifdef ENABLE_MQTT
        if (strcmp(pName, mqqtServerParam) == 0) {
          snprintf(WMconfig.WMmqttServer, sizeof(WMconfig.WMmqttServer), "%s", pValue);
        }
        if (strcmp(pName, mqqtPortParam) == 0) {
          WMconfig.WMmqttPort = p->value().toInt();
        }
        if (strcmp(pName, mqqtUserParam) == 0) {
          snprintf(WMconfig.WMmqttUser, sizeof(WMconfig.WMmqttUser), "%s", pValue);
        }
        if (strcmp(pName, mqqtPasswdParam) == 0) {
          snprintf(WMconfig.WMmqttPass, sizeof(WMconfig.WMmqttPass), "%s", pValue);
        }
        if (strcmp(pName, mqqtPrefixParam) == 0) {
          snprintf(WMconfig.WMmqttPrefix, sizeof(WMconfig.WMmqttPrefix), "%s", pValue);
        }
        
      #endif
    }
  }

  if (wifiSpremenjen) {
    write_Wifi_to_FS(wifiPath);
    DEBUG_PRINTLN("WiFi Setting saved to FS.");

    AsyncResponseStream *response = request->beginResponseStream("text/html");

    response->print(F("<!DOCTYPE html><html lang='sl'><head><meta charset='UTF-8'>"));
    response->print(F("<meta name='viewport' content='width=device-width, initial-scale=1.0'>"));
    response->print(F("<style>"));
    response->print(F("body{background-color:#1e222b; color:#fff; display:flex; justify-content:center; align-items:center; min-height:100vh; margin:0; padding:15px;}"));
    response->print(F(".box{background-color:#2d323f; padding:30px; border-radius:12px; box-shadow:0 10px 25px rgba(0,0,0,0.3); text-align:center; max-width:400px; width:100%;}"));
    response->print(F("h2{color:#4fd1c5; margin-bottom:15px; font-size:1.4rem;}"));
    response->print(F("p{color:#a0aec0; font-size:0.95rem; line-height:1.5;}"));
    response->print(F("</style></head><body>"));
    response->print(F("<div class='box'>"));
    response->print(F("<h2>Saved!</h2>"));
    response->print(F("<p>Omežje: <br><strong>"));
    
    response->print(WMconfig.WMssid);
    
    response->print(F("</strong></p>"));
    response->print(F("</div></body></html>"));

    request->send(response);

    WM_loop = millis();
    apActive = true;
    esp_reboot = true;

  } else {
    request->send(400, "text/plain", "Err: Wrong WifI data.");
  }
}


const char HTTP_SCRIPT[] PROGMEM =
  "<script>"
  "function c(l){var s=l.querySelector('span');document.getElementById('ssid_id').value=s.innerText||s.textContent;document.getElementById('pass_id').focus();}"
  "function togglePass(i){var x=document.getElementById(i);x.type=x.type==='password'?'text':'password';}"
  "function sproziInOsvezi(){var d=document.getElementById('wifi-list-container');if(d)d.innerHTML='<div>⏳ Iskanje omrežij...</div>';fetch('/scan-async').then(()=>setTimeout(osveziSeznam,5000));}"
  "function osveziSeznam(){var d=document.getElementById('wifi-list-container');fetch('/get-networks').then(r=>r.text()).then(h=>{if(d)d.innerHTML=h;});}"
  //"window.onload=function(){if(document.getElementById('wifi-list-container')){sproziInOsvezi();}};"
  "</script>";

const char HTTP_STYLE[] PROGMEM = 
  "<style>"
  "*{box-sizing:border-box;margin:0;padding:0;font-family:sans-serif;}"
  "body{background:#1e222b;color:#fff;padding:15px;display:flex;justify-content:center;align-items:center;min-height:100vh;}"
  ".container{width:100%;max-width:480px;}"
  ".info-card,form{background:#2d323f;box-shadow:0 10px 25px rgba(0,0,0,0.3);margin-bottom:10px;padding:10px;border-radius:12px;}"
  "h2,h3{font-size:1.2rem;}"
  "h3{font-size:1rem;text-transform:uppercase;color:#4fd1c5;padding-bottom:5px;}"
  ".info-card p{font-size:0.9rem;margin-bottom:8px;color:#a0aec0;}"
  ".form-row{display:flex;align-items:center;margin-bottom:12px;}"
  "label{font-size:0.9rem;width:130px;flex-shrink:0;}"
  ".mqform{color:#ed8936;}"
  ".input-container{flex-grow:1;position:relative;display:flex;flex-direction:column;}"
  "input[type='text'],input[type='password'],input[type='number']{background:#3f4453;border:1px solid transparent;border-radius:6px;color:#fff;padding:8px 12px;font-size:0.95rem;width:100%;outline:none;}"
  "input:focus{border-color:#63b3ed;}"
  ".show-pass{display:flex;align-items:center;justify-content:flex-end;gap:6px;margin-top:4px;}"
  ".show-pass label{width:auto;font-size:0.8rem;color:#a0aec0;}"
  "input[type='submit'],.scan-btn{background:#48bb78;color:#fff;border:none;border-radius:6px;padding:10px;font-size:1rem;font-weight:600;cursor:pointer;width:100%;text-transform:uppercase;text-align:center;display:block;text-decoration:none;}"
  "input[type='submit']:hover{background:#38a169;}"
  ".scan-btn{background:#4682b4;margin-top:15px;}"
  ".msg{margin-top:15px;font-size:0.8rem;color:#ed8936;text-align:center;background:rgba(255,255,255,0.05);padding:8px;border-radius:6px;}"
  ".link{background:#3f4453;color:#fff;margin:6px 0;padding:10px;display:block;border-radius:6px;text-decoration:none;font-size:0.9rem;}"
  "</style>";

void SendHTMLStream(AsyncResponseStream *response) {
   response->print(F("<!DOCTYPE html><html>\n<head>\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n<title>WiFi Manager</title>\n"));

  response->print(FPSTR(HTTP_SCRIPT));
  response->print(FPSTR(HTTP_STYLE));
  //response->print(FPSTR(HTTP_STYLE_HOME));

  response->print(F("<meta charset=\"UTF-8\">\n</head><body>\n"));
  response->print(F("<div class=\"container\">\n"));

  response->print(F("<div class=\"info-card\"><h2>"));
  response->print(clean_board_name);
  response->print(F(" | RSSI: "));
  response->print(getRSSIasQuality(WiFi.RSSI()));
  response->print(F(" %</h2></div>\n"));

  response->print(F("<form action=\""));
  response->print(serverURL_wifi_save);
  response->print(F("\" method=\"POST\" autocomplete=\"off\">\n<h3>WiFi</h3>\n"));

  response->print(F("<div class=\"form-row\"><label for=\"host_id\">Host name*</label><div class=\"input-container\"><input id=\"host_id\" required type=\"text\" name=\""));
  response->print(hostNameParam);
  response->print(F("\" value=\""));
  response->print(device_name);
  response->print(F("\" maxlength=\"20\" spellcheck=\"false\"></div></div>\n"));

  response->print(F("<div class=\"form-row\"><label for=\"ssid_id\">WiFi SSID</label><div class=\"input-container\"><input id=\"ssid_id\" required type=\"text\" name=\""));
  response->print(ssidParam);
  response->print(F("\" value=\""));
  response->print((strlen(WMconfig.WMssid) > 0) ? WMconfig.WMssid : hardCode_SSID);
  response->print(F("\" maxlength=\"32\" spellcheck=\"false\"></div></div>\n"));

  response->print(F("<div class=\"form-row\"><label for=\"pass_id\">WiFi Passwd</label><div class=\"input-container\"><input id=\"pass_id\" type=\"password\" name=\""));
  response->print(passParam);
  response->print(F("\" value=\""));
  response->print((strlen(WMconfig.WMpass) > 0) ? WMconfig.WMpass : hardCode_PASS);
  response->print(F("\" maxlength=\"64\"><div class=\"show-pass\"><input type=\"checkbox\" id=\"show_w_p\" onclick=\"togglePass('pass_id')\"><label for=\"show_w_p\">View</label></div></div></div>\n"));

#ifdef ENABLE_MQTT
  response->print(F("<h3 style=\"margin-top:20px; color:#ed8936;\">MQTT</h3>\n"));

  // MQTT Server
  response->print(F("<div class=\"form-row\"><label class=\"mqform\" for=\"mqs_id\">MQTT Server</label><div class=\"input-container\"><input id=\"mqs_id\" type=\"text\" name=\""));
  response->print(mqqtServerParam);
  response->print(F("\" value=\""));
  response->print(WMconfig.WMmqttServer);
  response->print(F("\" maxlength=\"32\" spellcheck=\"false\"></div></div>\n"));

  // MQTT Port
  response->print(F("<div class=\"form-row\"><label class=\"mqform\" for=\"mqpo_id\">MQTT Port</label><div class=\"input-container\"><input id=\"mqpo_id\" type=\"number\" name=\""));
  response->print(mqqtPortParam);
  response->print(F("\" value=\""));
  response->print(WMconfig.WMmqttPort);
  response->print(F("\" maxlength=\"6\"></div></div>\n"));

  // MQTT User
  response->print(F("<div class=\"form-row\"><label class=\"mqform\" for=\"mqu_id\">MQTT User</label><div class=\"input-container\"><input id=\"mqu_id\" type=\"text\" name=\""));
  response->print(mqqtUserParam);
  response->print(F("\" value=\""));
  response->print(WMconfig.WMmqttUser);
  response->print(F("\" maxlength=\"32\" spellcheck=\"false\"></div></div>\n"));

  // MQTT Password
  response->print(F("<div class=\"form-row\"><label class=\"mqform\" for=\"mqpw_id\">MQTT Passwd</label><div class=\"input-container\"><input id=\"mqpw_id\" type=\"password\" name=\""));
  response->print(mqqtPasswdParam);
  response->print(F("\" value=\""));
  response->print(WMconfig.WMmqttPass);
  response->print(F("\" maxlength=\"64\"><div class=\"show-pass\"><input type=\"checkbox\" id=\"show_m_p\" onclick=\"togglePass('mqpw_id')\"><label for=\"show_m_p\">Show</label></div></div></div>\n"));

  // MQTT Prefix
  response->print(F("<div class=\"form-row\"><label class=\"mqform\" for=\"mqu_pf\">MQTT prefix</label><div class=\"input-container\"><input id=\"mqu_id\" type=\"text\" name=\""));
  response->print(mqqtPrefixParam);
  response->print(F("\" value=\""));
  response->print(WMconfig.WMmqttPrefix);
  response->print(F("\" maxlength=\"32\" spellcheck=\"false\"></div></div>\n"));

#endif

  // SAVE
  response->print(F("<div style=\"margin-top:20px;\"><input type=\"submit\" value=\"SAVE\" accesskey=\"s\"></div>\n"));

  if (apActive == false) { 
    response->print(F("<div class=\"msg\">⚠️ Manual restart required for MQTT changes.</div>\n")); 
  }

  response->print(F("</form>\n"));

  if (apActive == true) {
    response->print(F("<form class=\"scan-form\">\n<h3 style=\"color:#4aa3df;\">WiFi in range</h3>\n"));
    response->print(F("<div id=\"wifi-list-container\"><div style=\"color:#a0aec0; font-size:0.9rem; text-align:center; padding:10px;\">Pritisnite REFRESH za iskanje omrežij.</div></div>\n"));
    response->print(F("<a href=\"#\" class=\"scan-btn\" onclick=\"sproziInOsvezi(); return false;\">Refresh</a>\n"));
    response->print(F("</form>\n"));
  }

  response->print(F("</div></div>\n</body></html>\n"));
}


void write_Wifi_to_FS(const char *path) {

  File file = LittleFS.open(path, "w");
  if (!file) {
    DEBUG_PRINTLN("Failed to open file for writing");
    return;
  }
  if (file.print(parseWifi())) {
    DEBUG_PRINTLN("File written");
  } else {
    DEBUG_PRINTLN("Write failed");
  }
  file.close();
  DEBUG_PRINTLN("writeFile closed");

  if (apActive == true) {
    esp_reboot = true;
  } else if (apActive == false) {
    manual_reboot_req = true;
    notifyClients(notify_evt, ESPinfo().c_str());
  }
  //readWifi_FS();
}

String parseWifi() {
  JsonDocument jsonBuffer;
  JsonObject root         = jsonBuffer.to<JsonObject>();

  root[hostNameParam]     = WMconfig.WMhostName;
  root[ssidParam]         = WMconfig.WMssid;
  root[passParam]         = WMconfig.WMpass;

  #ifdef ENABLE_MQTT
    root[mqqtServerParam] = WMconfig.WMmqttServer;
    root[mqqtPortParam]   = WMconfig.WMmqttPort;
    root[mqqtUserParam]   = WMconfig.WMmqttUser;
    root[mqqtPasswdParam] = WMconfig.WMmqttPass;
    root[mqqtPrefixParam] = WMconfig.WMmqttPrefix;
  #endif

  String output = "";
  output.reserve(512);
  
  serializeJson(jsonBuffer, output);

  DEBUG_PRINTLN(output.c_str());

  return output;
}



bool read_Wifi_from_FS() {
  if (LittleFS.exists(wifiPath)) {
    DEBUG_PRINTLN("Reading WIFI config file... ");
    File configFile = LittleFS.open(wifiPath, "r");

    if (configFile) {
      DEBUG_PRINTLN("WIFI File Opened!");
      JsonDocument json;

      DeserializationError error = deserializeJson(json, configFile);
      configFile.close();

      if (!error) {
        DEBUG_PRINTLN("WIFI Parsed successfully!");

        if (json[hostNameParam].is<const char *>()) {
          strncpy(device_name, json[hostNameParam].as<const char *>(), sizeof(device_name) - 1);
          device_name[sizeof(device_name) - 1] = '\0';
        }

        WMconfig.WMhostName     = device_name;

        snprintf(WMconfig.WMssid, sizeof(WMconfig.WMssid), "%s", json[ssidParam].as<const char*>());
        snprintf(WMconfig.WMpass, sizeof(WMconfig.WMpass), "%s", json[passParam].as<const char*>());

        #ifdef ENABLE_MQTT
          snprintf(WMconfig.WMmqttServer, sizeof(WMconfig.WMmqttServer), "%s", json[mqqtServerParam].as<const char*>());
          WMconfig.WMmqttPort   = json[mqqtPortParam].as<uint16_t>(); // Številke lahko ostanejo z '='
          snprintf(WMconfig.WMmqttUser, sizeof(WMconfig.WMmqttUser), "%s", json[mqqtUserParam].as<const char*>());
          snprintf(WMconfig.WMmqttPass, sizeof(WMconfig.WMmqttPass), "%s", json[mqqtPasswdParam].as<const char*>());
          snprintf(WMconfig.WMmqttPrefix, sizeof(WMconfig.WMmqttPrefix), "%s", json[mqqtPrefixParam].as<const char*>());
        #endif


        return true;
      } else {
        DEBUG_PRINTLN("Failed to load WManager json config, parsing error.");
        apActive = true;
      }
    } else {
      DEBUG_PRINTLN("Failed to open /wifi.json");
    }
  } else {
    DEBUG_PRINTLN("Could not find wifi.json");
  }
  return false;
}

const char HTTP_ITEM[] PROGMEM = "<div><a href='#p' class='link' onclick='c(this)'><span>{n}</span><span style='float:right; color:#3cb371;'>{s}</span><div style='clear:both;'></div></a></div>";


void scanWifi_SSID(AsyncResponseStream *response, int n) {
  if (n <= 0) {
    response->print(F("<div>No Wi-Fi in range.</div>"));
    return;
  }

  int indices[n];
  for (int i = 0; i < n; i++) indices[i] = i;

  // Sortiranje po RSSI
  for (int i = 0; i < n; i++) {
    for (int j = i + 1; j < n; j++) {
      if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
        std::swap(indices[i], indices[j]);
      }
    }
  }

  // Filtriranje duplikatov
  for (int i = 0; i < n; i++) {
    if (indices[i] == -1) continue;
    String cssid = WiFi.SSID(indices[i]);
    for (int j = i + 1; j < n; j++) {
      if (cssid == WiFi.SSID(indices[j])) {
        indices[j] = -1;
      }
    }
  }

  for (int i = 0; i < n; i++) {
    if (indices[i] == -1) continue;
    if (WiFi.SSID(indices[i]).length() == 0) continue;

    int32_t rssi = WiFi.RSSI(indices[i]);
    int quality = (rssi <= -100) ? 0 : (rssi >= -50) ? 100 : 2 * (rssi + 100);

    response->print(F("<a href='#' class='link' onclick='c(this); return false;'>📶 <span>"));
    response->print(WiFi.SSID(indices[i]));
    response->print(F("</span> ("));
    response->print(quality);
    response->print(F("%)</a>\n"));
  }
}

