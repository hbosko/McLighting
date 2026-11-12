#include "notFound_HTML.h"
#include "brd_settings_HTML.h"
#include "esp_Control_panel_HTML.h"

void custom_webserver_path(){
  #ifdef ENABLE_UDP
    serverOn_UDP();
  #endif

/* 
  server.on("/px/modes", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginChunkedResponse("application/json", [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
      static uint16_t currentMode = 0;
      
      if (index == 0) {
        currentMode = 0;
      }

      uint16_t modeCount = strip->getModeCount();
      if (currentMode >= modeCount) {
        return 0;
      }

      String chunk = "";
      if (currentMode == 0) {
          chunk += "[";
      }

      chunk += "{\"id\":" + String(currentMode) + ",\"nm\":\"" + String(strip->getModeName(currentMode)) + "\"}";
      currentMode++;

      if (currentMode < modeCount) {
          chunk += ",";
      } else {
          chunk += "]";
      }

      size_t len = chunk.length();
      if (len > maxLen) {
          len = maxLen;
      }
      memcpy(buffer, chunk.c_str(), len);
      return len;
    }, NULL);

    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response);
  });
*/
    // OOM!
  server.on(serverURL_getModes, HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    //palette_json(*response); //FASTLED
    listModesJSON(*response); //ws2812
    request->send(response);
  });
    
    //save strip settings
  server.on(serverURL_strip_state, HTTP_POST, handle_strip_state);

  server.on(serverURL_edit_palettes, HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->client() != nullptr) {
      AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", palette_editor_html_gz, palette_editor_html_gz_len);
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);
    }
  });

    server_builtin_palettes();
    handler_save_palettes();

  /* // websockets faster.
    server.on("/set_brightness", HTTP_GET, [](AsyncWebServerRequest * request) {
      if (request->hasParam("v")) {
        int val = request->getParam("v")->value().toInt();
        
        if (val >= 0 && val <= 255) {
          brightness = val;
          
          if (strip) {
            strip->setBrightness(brightness);
            strip->trigger();
          }

            save_strip_state_ticker.detach();
            save_strip_state_ticker.once(save_ticker_state_time, Trigger_Save_State);
        }
      }

      request->send(200, "application/json", "{\"status\":\"success\"}");
    });

    server.on("/get_brightness", HTTP_GET, [](AsyncWebServerRequest * request) {
      char buf[4];
      snprintf(buf, sizeof(buf), "%d", (int)(brightness / 2.55));
      request->send(200, "text/plain", buf);
      
          save_strip_state_ticker.detach();
          save_strip_state_ticker.once(save_ticker_state_time, Trigger_Save_State);

      #ifdef DEBUG
        DEBUG_PRINTLN("/get_brightness: ");
        DEBUG_PRINTLN(buf);
      #endif
    });

    server.on("/set_speed", HTTP_POST, [](AsyncWebServerRequest * request) {
      if (request->hasParam("d", true)) {
        int speedVal = request->getParam("d", true)->value().toInt();
        ws2812fx_speed = constrain(speedVal, 0, 255);
        
        if (strip) {
          strip->setSpeed(convertSpeed(ws2812fx_speed));
          strip->trigger();
        }

        #ifdef DEBUG
          DEBUG_PRINT("ws2812fx_speed set to: ");
          DEBUG_PRINTLN(ws2812fx_speed);
        #endif

          save_strip_state_ticker.detach();
          save_strip_state_ticker.once(save_ticker_state_time, Trigger_Save_State);
      }
      request->send(200, "text/plain", "OK");
    });

    server.on("/get_speed", HTTP_GET, [](AsyncWebServerRequest * request) {
      char buf[4];
      snprintf(buf, sizeof(buf), "%d", ws2812fx_speed);
      request->send(200, "text/plain", buf);
      
      #ifdef DEBUG
        DEBUG_PRINTLN("/get_speed: ");
        DEBUG_PRINTLN(buf);
      #endif
    });

    server.on("/get_switch", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(200, "text/plain", (Mode == OFF) ? "0" : "1");
    });

    server.on("/get_color", HTTP_GET, [](AsyncWebServerRequest * request) {
      char rgbcolor[8]; 
      snprintf(rgbcolor, sizeof(rgbcolor), "%02X%02X%02X", main_color.red, main_color.green, main_color.blue);
      request->send(200, "text/plain", rgbcolor);
      
          save_strip_state_ticker.detach();
          save_strip_state_ticker.once(save_ticker_state_time, Trigger_Save_State);
      #ifdef DEBUG
        DEBUG_PRINTLN("/get_color: ");
        DEBUG_PRINTLN(rgbcolor);
      #endif
    });
  */

}


void init_WEB_server(){
  HTTP_enabled = 1;

    //init_global_login();

    server.on(wifiPath, HTTP_ANY, [](AsyncWebServerRequest *request){
      //request->send(403, "text/plain", "Downloading Virus...");
      request->redirect("/login");

    });

  #if include_indexHTML == 1

    server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
      if (request->client() != nullptr) {
        HTTP_client = request->client()->remoteIP().toString();
        //  uint16_t remotePort = request->client()->remotePort();
        //  String localIP = request->client()->localIP().toString();
      }

      AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html_gz, index_html_gz_len);
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);
    });

  #else
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      if (request->client() != nullptr) {
        HTTP_client = request->client()->remoteIP().toString();
      }

    if (LittleFS.exists("/index.html")) {
      request->send(LittleFS, "/index.html", "text/html");
    }
    else if (LittleFS.exists("/index.html.gz")) {
      AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/index.html.gz", "text/html");
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);
    } else {
      request->redirect("/login");
      DEBUG_PRINTLN("NO index.html redirecting...");
    }
  });

  #endif

  server.on(serverURL_ESP_info, HTTP_GET, [](AsyncWebServerRequest * request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    if (response) {
      String jsonBuffer;
      jsonBuffer.reserve(1024);
        ESPinfo(jsonBuffer);
      response->print(jsonBuffer);
      request->send(response);
    } else {
      request->send(500, "text/plain", "Neuspešna alokacija streama");
    }
  });

  /*
    server.on(serverURL_ESP_info, HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(200, "application/json", ESPinfo());
    });
  */
  server.on(serverURL_ESP_CP, HTTP_ANY, [](AsyncWebServerRequest * request) {
    /*
    if (!request->authenticate(login_username, login_password)) {
      return request->requestAuthentication();
    }*/
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", esp_cp_html_gz, esp_cp_html_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

    // board settings
  server.on(serverURL_BRD_sett_json, HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    brd_json(*response);
    request->send(response);
  });
  server.on(serverURL_BRD_sett_save, HTTP_ANY, handle_esp_settings_save);

  server.on(serverURL_BRD_sett_html, HTTP_GET, [](AsyncWebServerRequest * request) {
      Settings_client = request->client()->remoteIP().toString();
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", brd_settings_html_gz, brd_settings_html_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    response->addHeader("Pragma", "no-cache");
    response->addHeader("Expires", "-1");
    request->send(response);
  });

    // scanners
  server.on(serverURL_scan_wifi, HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    scanWifi(*response);
    request->send(response);
  });

  server.on(serverURL_scan_i2c, HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    scan_i2c(*response); 
    request->send(response);
  });

  /* // dont remove
    server.on(serverURL_scan_OneWire, HTTP_GET, [](AsyncWebServerRequest *request) {
      //request->send(200, "application/json", scan_1wire());
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      scan_1wire(*response);
      request->send(response);
    });
  */
    custom_webserver_path();

    //LEAVE this here!!
    server.serveStatic("/", LittleFS, "/");

  server.onNotFound([](AsyncWebServerRequest *request){
    if (request->client() != nullptr) {
      NotFound_client = request->client()->remoteIP().toString();
    }

    AsyncWebServerResponse *response = request->beginResponse_P(404, "text/html", notFound_html_gz, notFound_html_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    response->addHeader("Pragma", "no-cache");
    response->addHeader("Expires", "-1");
    request->send(response);
    DEBUG_PRINTF(" 404 od [%s] -> http://%s%s\n", NotFound_client.c_str(), request->host().c_str(), request->url().c_str());
  });


  // WEB_Start server
  server.begin();
  DEBUG_PRINTLN("WEB server started!");
}


void WIFIMANAGER_server_path(){

  server.on(serverURL_edit_ap, HTTP_ANY, [](AsyncWebServerRequest *request) {
    if (request->client() != nullptr) {
      wifi_edit_client = request->client()->remoteIP().toString();
    }

  /*
    if (client_loged_out >= 1) {
      client_loged_out = 0;
      AsyncWebServerResponse *response = request->beginResponse(401, "text/plain", "Odjava uspešna.");
      response->addHeader("WWW-Authenticate", "Basic realm='Logout'");
      request->send(response);
      DEBUG_PRINTLN("Served AP LOGOUT Page (Cleared browser session)");
      return;
    }

    if (firstBoot == false) {
      if (!request->authenticate(edit_AP_username, edit_AP_password)) {
        return request->requestAuthentication();
      }
    }
  */
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", SendHTML());
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    response->addHeader("Pragma", "no-cache");
    response->addHeader("Expires", "-1");
    request->send(response);
    DEBUG_PRINTLN("Served AP EDIT Page");
  });

  server.on(serverURL_ESP_cmd, HTTP_POST, handle_wm_cmd);
  server.on(serverURL_wifi_save, HTTP_POST, handle_wifisave);
}
