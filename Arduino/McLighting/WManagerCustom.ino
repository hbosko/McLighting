// https://github.com/CDFER/Captive-Portal-ESP32

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



void initAP_path(){

  server.on("/connecttest.txt", HTTP_ANY, [](AsyncWebServerRequest *request) { request->redirect("http://logout.net");});
  server.on("/connecttest.txt", HTTP_ANY, [](AsyncWebServerRequest *request) { request->send(200, "text/plain", "Microsoft Connect Test"); });
  server.on("/wpad.dat", HTTP_ANY, [](AsyncWebServerRequest *request) { request->send(404); });
  server.on("/ncsi.txt", HTTP_ANY, [](AsyncWebServerRequest *request) { request->send(200, "text/plain", "Microsoft NCSI"); });

  server.on("/scan-async", HTTP_GET, [](AsyncWebServerRequest *request) {
    #if defined(ESP8266)
      WiFi.mode(WIFI_AP_STA);
      delay(10);
      WiFi.scanDelete();
      WiFi.scanNetworks(true);
    #elif defined(ESP32)
      WiFi.mode(WIFI_AP_STA);
      WiFi.scanDelete();
      WiFi.scanNetworks(true, true); 
    #endif
    request->send(200, "text/plain", "OK");
  });

  server.on("/get-networks", HTTP_GET, [](AsyncWebServerRequest *request) {
    int n = WiFi.scanComplete();
    
    if (n == -1) {
      request->send(200, "text/html", "<div>⏳ Refreshing... <br><br><a href='#' class='scan-btn' onclick='osveziSeznam(); return false;'>SHOW WiFis</a></div>");
    } else if (n == -2 || n == 0) {
      request->send(200, "text/html", "<div>❌ No WiFi in range. <br><br><a href='#' class='scan-btn' onclick='sproziInOsvezi(); return false;'>SCAN AGAIN</a></div>");
    } else {
      request->send(200, "text/html", scanWifi_SSID());
      #if defined(ESP8266)
        WiFi.mode(WIFI_AP); 
        wifi_station_disconnect(); 
      #endif
    }
  });

  server.on("/generate_204", HTTP_ANY, [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });
  server.on("/gen_204", HTTP_ANY, [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });
  server.on("/redirect", HTTP_ANY, [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });
  server.on("/hotspot-detect.html", HTTP_ANY, [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });
  server.on("/canonical.html", HTTP_ANY, [](AsyncWebServerRequest *request) { request->redirect(localIPURL); });
  server.on("/success.txt", HTTP_ANY, [](AsyncWebServerRequest *request) { request->send(200, "text/plain", "success"); });
  server.on("/favicon.ico", HTTP_ANY, [](AsyncWebServerRequest *request) { request->send(404); });

  server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request) {
    /*
    if (firstBoot == false) {
      if(!request->authenticate(edit_AP_username, edit_AP_password))
        return request->requestAuthentication();
    }
    */
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", SendHTML());
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    response->addHeader("Pragma", "no-cache");
    response->addHeader("Expires", "-1");
    request->send(response);
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    String host = request->host();
    if (host.indexOf("4.4.4.1") == -1 && (request->url().indexOf(".html") != -1 || request->url() == "/")) {
      request->redirect(localIPURL);
    } else {
      request->send(404);
    }
  });
}




void connectToWifi() {
  WiFi.mode(WIFI_STA);

  #if defined(ESP32)
  //https://forum.arduino.cc/t/no-wifi-connect-with-esp32-c3-super-mini/1324046/21
    //WiFi.setTxPower(WIFI_POWER_8_5dBm);
  #endif

  DEBUG_PRINTLN("Connecting to Wi-Fi...");
  WiFi.begin(WMconfig.WMssid, WMconfig.WMpass);

  DEBUG_PRINT("WMssid: ");
  DEBUG_PRINTLN(WMconfig.WMssid);

  DEBUG_PRINT("AP active: ");
  DEBUG_PRINTLN(apActive);

}

#if defined(ESP8266)

  void onWifiConnect(const WiFiEventStationModeGotIP& event) {
    (void)event;
      AP_count=0;

    WiFi.mode(WIFI_STA);
    WiFi.softAPdisconnect(true);
      apActive = false;

    DEBUG_PRINTLN("Connected to Wi-Fi.");
    DEBUG_PRINT("IP address 82: ");
    DEBUG_PRINTLN(WiFi.localIP());
    #ifdef ENABLE_MQTT
      connectToMqtt();
    #endif

  }

  void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
    (void)event;
    if (!apActive) {
      AP_count++;
      DEBUG_PRINT("AP AP_count: ");
      DEBUG_PRINTLN(AP_count);

      if (AP_count >= 10) {
        apActive = true;
        startSoftAccessPoint(softAPname.c_str(), softAP_PASS, localIP, gatewayIP);
      }
    }
    DEBUG_PRINTLN("Disconnected from Wi-Fi.");
  }

#elif defined(ESP32)

  void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
    DEBUG_PRINTLN("Connected to AP OK!");
    //WiFi.mode(WIFI_STA);
    WiFi.softAPdisconnect(true);
      apActive = false;
  }

  void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
    DEBUG_PRINTLN("WiFi connected");
    DEBUG_PRINT("WiFiGotIP IP address: ");
      DEBUG_PRINTLN(WiFi.localIP());
  }

  void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
    DEBUG_PRINTLN("Disconnected from WiFi access point");
    DEBUG_PRINT("WiFi lost connection. Reason: ");
    DEBUG_PRINTLN(info.wifi_sta_disconnected.reason);
    DEBUG_PRINTLN("Trying to Reconnect");
    AP_count++;

    DEBUG_PRINTLN("AP_count: ");
    DEBUG_PRINTLN(AP_count);

    if (AP_count >= 10) {
      apActive = true;
      startSoftAccessPoint(softAPname.c_str(), softAP_PASS, localIP, gatewayIP);
    } else if (AP_count == 120) {
      esp_reboot = true;
    }
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

    startSoftAccessPoint(softAPname.c_str(), softAP_PASS, localIP, gatewayIP);

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

    #elif defined(ESP32) // mqtt not connecting...
    //  WiFi.onEvent(WiFiStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
      WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
      //WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
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

  if (millis() - WM_loop > 5000) {
    WM_loop = millis();

    if (apActive == false && WiFi.status() != WL_CONNECTED) {
      connectToWifi();
      WIFI_connected = 0;
    }
    else if (apActive == false && WiFi.status() == WL_CONNECTED) {
      WIFI_connected = 1;
    }

    if (apActive == true) {
      AP_count++;
      #if defined(ESP8266)
        if (WiFi.status() == WL_DISCONNECTED) {
          wifi_station_disconnect(); 
        }
      #endif

      if (AP_count == 120) {
        esp_reboot = true;
      }
    }

    #ifdef ENABLE_MQTT
      if (!apActive && WiFi.status() == WL_CONNECTED && !MQTT_connected) {
        DEBUG_PRINTLN( "MQTT disconnecteD.. trying... hard" );
        connectToMqtt();
      }
    #endif
  }
}

void handle_wm_cmd(AsyncWebServerRequest* request) {
  int params = request->params();
  
  String odzivStatus = "NAPACNO GESLO";

    for(int i=0;i<params;i++){
      const AsyncWebParameter* p = request->getParam(i);
        DEBUG_PRINTF("Param name: %s | value: %s\r\n", p->name().c_str(), p->value().c_str());

      if (p->name() == "rbt") {
        if (p->value() == "1") {
          notifyClients(notify_evt,notify_OK);
        odzivStatus = "OK - Rebooting...";
          esp_reboot = true;
        } else {
          notifyClients(notify_evt,notify_FAILED);
        }

      }

      if (p->name() == "wm") {
        if (p->value() == "1") {
          notifyClients(notify_evt,"Reboot and switch to AP mode");
            odzivStatus = "OK - Switching to AP Mode... and RebooT";
          deleteFile(wifiPath);
          esp_reboot = true;
        } else {
          notifyClients(notify_evt,notify_FAILED);
        }

      }

    }

  AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", odzivStatus);
  response->addHeader("Access-Control-Allow-Origin", "*");
  request->send(response);
}


void handle_wifisave(AsyncWebServerRequest* request) {
  int params = request->params();
  bool wifiSpremenjen = false;
  
  for (int i = 0; i < params; i++) {
    const AsyncWebParameter* p = request->getParam(i);
      DEBUG_PRINTF("Param name: %s | value: %s\r\n", p->name().c_str(), p->value().c_str());

    if (p->isPost() || request->method() == HTTP_POST) {
      if (p->name() == hostNameParam) {
        strncpy(device_name, p->value().c_str(), sizeof(device_name) - 1);
        device_name[sizeof(device_name) - 1] = '\0';
        device_name_c = device_name;
        WMconfig.WMhostName = device_name;
        wifiSpremenjen = true;
      }
      
      if (p->name() == ssidParam) {
        WMconfig.WMssid = p->value();
        wifiSpremenjen = true;
      }
      
      if (p->name() == passParam) {
        WMconfig.WMpass = p->value();
        wifiSpremenjen = true;
      }

      #ifdef ENABLE_MQTT
        if (p->name() == mqqtServerParam) {
          WMconfig.WMmqttServer = p->value();
        }
        if (p->name() == mqqtPortParam) {
          WMconfig.WMmqttPort = p->value().toInt();
        }
        if (p->name() == mqqtUserParam) {
          WMconfig.WMmqttUser = p->value();
        }
        if (p->name() == mqqtPasswdParam) {
          WMconfig.WMmqttPass = p->value();
        }
      #endif
    }
  }

  if (wifiSpremenjen) {

    write_Wifi_to_FS(wifiPath); 

    DEBUG_PRINTLN("Setting saved to FS.");

    String uspeh = "<!DOCTYPE html><html lang='sl'><head><meta charset='UTF-8'>";
    uspeh += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    uspeh += "<style>";
    uspeh += "body{background-color:#1e222b; color:#fff; display:flex; justify-content:center; align-items:center; min-height:100vh; margin:0; padding:15px;}";
    uspeh += ".box{background-color:#2d323f; padding:30px; border-radius:12px; box-shadow:0 10px 25px rgba(0,0,0,0.3); text-align:center; max-width:400px; width:100%;}";
    uspeh += "h2{color:#4fd1c5; margin-bottom:15px; font-size:1.4rem;}";
    uspeh += "p{color:#a0aec0; font-size:0.95rem; line-height:1.5;}";
    uspeh += "</style></head><body>";
    uspeh += "<div class='box'>";
    uspeh += "<h2>Saved!</h2>";
    uspeh += "<p>Omežje: <br><strong>" + WMconfig.WMssid + "</strong></p>";
    uspeh += "</div></body></html>";

    request->send(200, "text/html", uspeh);

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
"</script>";


const char HTTP_STYLE[] PROGMEM = "<style>"
"*{box-sizing:border-box;margin:0;padding:0;font-family:sans-serif;}"
"body{background:#1e222b;color:#fff;padding:15px;display:flex;justify-content:center;align-items:center;min-height:100vh;}"
".container{width:100%;max-width:480px;}"
".info-card,form{background:#2d323f;box-shadow:0 10px 25px rgba(0,0,0,0.3);margin-bottom:10px;padding:10px;border-radius:12px;}"
"h2,h3{font-size:1.2rem;}"
"h3{font-size:1rem;text-transform:uppercase;color:#4fd1c5;border-bottom:1px solid rgba(255,255,255,0.1);padding-bottom:5px;}"
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

String SendHTML(){
  String html = "<!DOCTYPE html><html>\n";
  html +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  html +="<title>WiFi Manager</title>\n";
  html += FPSTR(HTTP_SCRIPT);
  html += FPSTR(HTTP_STYLE);
  html +="<meta charset=\"UTF-8\">\n";   
  html +="</head><body>\n";
  html +="<div class=\"container\">\n";

  html +="<div class=\"info-card\">\n";
  html +="<h2>";
  html += getCleanBoardName();
  html +=" | RSSI: ";
  html += String(getRSSIasQuality(WiFi.RSSI())) + " %";
  html +="</h2></div>\n";

  html +="<form action=\"";
  html += serverURL_wifi_save;
  html +="\" method=\"POST\" autocomplete=\"off\">\n";
  html +="<h3>WiFi Settings</h3>\n";

  // Host Name
  html +="<div class=\"form-row\">";
  html +="<label for=\"host_id\">Host name*</label>";
  html +="<div class=\"input-container\"><input id=\"host_id\" required type=\"text\" name=\"";
  html += hostNameParam;
  html +="\" value=\"";
  html += device_name;
  html +="\" maxlength=\"16\" spellcheck=\"false\"></div>";
  html +="</div>\n";
  
  // SSID
  html +="<div class=\"form-row\">";
  html +="<label for=\"ssid_id\">WiFi SSID</label>";
  html +="<div class=\"input-container\"><input id=\"ssid_id\" required type=\"text\" name=\"";
  html += ssidParam;
  html +="\" value=\"";
  html += ((WMconfig.WMssid.length() > 0) ? WMconfig.WMssid : hardCode_SSID);
  html +="\" maxlength=\"32\" spellcheck=\"false\"></div>";
  html +="</div>\n";
  
  // WiFi Password
  html +="<div class=\"form-row\">";
  html +="<label for=\"pass_id\">WiFi Passwd</label>";
  html +="<div class=\"input-container\">";
  html +="<input id=\"pass_id\" type=\"password\" name=\"";
  html += passParam;
  html +="\" value=\"";
  html += ((WMconfig.WMpass.length() > 0) ? WMconfig.WMpass : hardCode_PASS);
  html +="\" maxlength=\"64\">";
  html +="<div class=\"show-pass\"><input type=\"checkbox\" id=\"show_w_p\" onclick=\"togglePass('pass_id')\"><label for=\"show_w_p\">View</label></div>";
  html +="</div>";
  html +="</div>\n";

  #ifdef ENABLE_MQTT
    html +="<h3 style=\"margin-top:20px; color:#ed8936;\">MQTT Settings</h3>\n";
    
    // MQTT Server
    html +="<div class=\"form-row\">";
    html +="<label class=\"mqform\" for=\"mqs_id\">MQTT Server</label>";
    html +="<div class=\"input-container\"><input id=\"mqs_id\" type=\"text\" name=\"";
    html += mqqtServerParam;
    html +="\" value=\"";
    html += WMconfig.WMmqttServer;
    html +="\" maxlength=\"32\" spellcheck=\"false\"></div>";
    html +="</div>\n";
    
    // MQTT Port
    html +="<div class=\"form-row\">";
    html +="<label class=\"mqform\" for=\"mqpo_id\">MQTT Port</label>";
    html +="<div class=\"input-container\"><input id=\"mqpo_id\" type=\"number\" name=\"";
    html += mqqtPortParam;
    html +="\" value=\"";
    html += String(WMconfig.WMmqttPort);
    html +="\" maxlength=\"6\"></div>";
    html +="</div>\n";

    // MQTT User
    html +="<div class=\"form-row\">";
    html +="<label class=\"mqform\" for=\"mqu_id\">MQTT User</label>";
    html +="<div class=\"input-container\"><input id=\"mqu_id\" type=\"text\" name=\"";
    html += mqqtUserParam;
    html +="\" value=\"";
    html += WMconfig.WMmqttUser;
    html +="\" maxlength=\"32\" spellcheck=\"false\"></div>";
    html +="</div>\n";

    // MQTT Password
    html +="<div class=\"form-row\">";
    html +="<label class=\"mqform\" for=\"mqpw_id\">MQTT Passwd</label>";
    html +="<div class=\"input-container\">";
    html +="<input id=\"mqpw_id\" type=\"password\" name=\"";
    html += mqqtPasswdParam;
    html +="\" value=\"";
    html += WMconfig.WMmqttPass;
    html +="\" maxlength=\"64\">";
    html +="<div class=\"show-pass\"><input type=\"checkbox\" id=\"show_m_p\" onclick=\"togglePass('mqpw_id')\"><label for=\"show_m_p\">Show</label></div>";
    html +="</div>";
    html +="</div>\n";
  #endif

  html += "<div style=\"margin-top:20px;\"><input type=\"submit\" value=\"SAVE\" accesskey=\"s\"></div>\n";   

  if(apActive == false) {
    html += "<div class=\"msg\">⚠️ Manual restart required for MQTT changes.</div>\n";
  }
  html += "</form>\n";   

  if(apActive == true) {
    html += "<form class=\"scan-form\">\n";
    html += "<h3 style=\"color:#4aa3df;\">WiFi in range</h3>\n";
    
    html += "<div id=\"wifi-list-container\">";
    html += "</div>\n";
    
    html += "<a href=\"#\" class=\"scan-btn\" onclick=\"osveziSeznam(); return false;\">Refresh</a></div>\n";
    html += "</form>\n";
  }

  html += "</div>\n";
  html += "</body></html>\n";
  return html;
}



void write_Wifi_to_FS(const char* path) {

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

    if( apActive == true) {
      esp_reboot = true;
    }
    else if( apActive == false) {
      manual_reboot_req = true;
      notifyClients(notify_evt,ESPinfo());
    }
  //readWifi_FS();
}


String parseWifi() {
  JsonDocument jsonBuffer;
  JsonObject root = jsonBuffer.to<JsonObject>();

  root[hostNameParam]   = WMconfig.WMhostName;
  root[ssidParam]       = WMconfig.WMssid;
  root[passParam]       = WMconfig.WMpass;
  
  #ifdef ENABLE_MQTT
    root[mqqtServerParam] = WMconfig.WMmqttServer;
    root[mqqtPortParam]   = WMconfig.WMmqttPort;
    root[mqqtUserParam]   = WMconfig.WMmqttUser;
    root[mqqtPasswdParam] = WMconfig.WMmqttPass;
  #endif

  String output = "";
  serializeJson(jsonBuffer, output);
  
  DEBUG_PRINTLN(output);
  
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

        if (json[hostNameParam].is<const char*>()) {
          strncpy(device_name, json[hostNameParam].as<const char*>(), sizeof(device_name) - 1);
          device_name[sizeof(device_name) - 1] = '\0';
        }
        
        device_name_c = device_name;
        WMconfig.WMhostName = device_name;

        WMconfig.WMssid = json[ssidParam].as<String>();
        WMconfig.WMpass = json[passParam].as<String>();

        #ifdef ENABLE_MQTT
          WMconfig.WMmqttServer = json[mqqtServerParam].as<String>();
          WMconfig.WMmqttPort   = json[mqqtPortParam].as<uint16_t>();
          WMconfig.WMmqttUser   = json[mqqtUserParam].as<String>();
          WMconfig.WMmqttPass   = json[mqqtPasswdParam].as<String>();
        #endif

        return true;
      } else {
        DEBUG_PRINTLN("Failed to load WManager json config due to parsing error.");
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

String scanWifi_SSID() {
  int n = WiFi.scanComplete();
  String output = "";
  
  if (n <= 0) return "<div>No Wi-Fi in range.</div>";

  int indices[n];
  for (int i = 0; i < n; i++) indices[i] = i;

  // Sort (RSSI)
  for (int i = 0; i < n; i++) {
    for (int j = i + 1; j < n; j++) {
      if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
        std::swap(indices[i], indices[j]);
      }
    }
  }

  // Filter duplicates
  String cssid;
  for (int i = 0; i < n; i++) {
    if (indices[i] == -1) continue;
    cssid = WiFi.SSID(indices[i]);
    for (int j = i + 1; j < n; j++) {
      if (cssid == WiFi.SSID(indices[j])) {
        indices[j] = -1; 
      }
    }
  }

  String HTTP_ITEM_STR = FPSTR(HTTP_ITEM);

  for (int i = 0; i < n; i++) {
    if (indices[i] == -1) continue;
    if (WiFi.SSID(indices[i]).length() == 0) continue;

    String item = HTTP_ITEM_STR;
    String ss_id = WiFi.SSID(indices[i]);
    int32_t rssi = WiFi.RSSI(indices[i]);
    
    int quality = (rssi <= -100) ? 0 : (rssi >= -50) ? 100 : 2 * (rssi + 100);
    String qll  = String(quality) + "%";
    /*
      String signalIcon = "";
      if (quality > 80)       signalIcon = "||||  " + String(quality) + "%";
      else if (quality > 60)  signalIcon = "|||   " + String(quality) + "%";
      else if (quality > 40)  signalIcon = "||    " + String(quality) + "%";
      else                    signalIcon = "|     " + String(quality) + "%";
    */
    item.replace("{n}", ss_id);
    item.replace("{s}", qll);
    //item.replace("{s}", signalIcon);
    output += item;
  }
  
  return output;
}

