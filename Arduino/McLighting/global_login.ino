const char login_html[] PROGMEM = "<!DOCTYPE html><html lang=\"sl\"><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"><style>*{box-sizing:border-box;margin:0;padding:0}body{background:#09090b;color:#f4f4f5;font-family:-apple-system,BlinkMacSystemFont,\"Segoe UI\",Roboto,sans-serif;display:flex;align-items:center;justify-content:center;min-height:100vh;padding:20px}.card{background:#18181b;border:1px solid #27272a;border-radius:12px;padding:32px;width:100vw;max-width:360px;box-shadow:0 4px 20px rgba(0,0,0,.4)}h3{font-size:1.25rem;font-weight:600;margin-bottom:24px;color:#fff;letter-spacing:-.025em}input{width:100%%;padding:12px 14px;margin-bottom:16px;border-radius:6px;border:1px solid #3f3f46;background:#09090b;color:#fff;font-size:.95rem;transition:all .15s ease;outline:none}input:focus{border-color:#06b6d4;box-shadow:0 0 0 2px rgba(6,182,212,.15)}button{width:100%%;padding:12px;border-radius:6px;border:none;background:#06b6d4;color:#09090b;font-weight:600;cursor:pointer;font-size:.95rem;transition:background .15s ease}button:hover{background:#22d3ee}.error{background:rgba(239,68,68,.1);border:1px solid rgba(239,68,68,.2);color:#f87171;padding:10px;border-radius:6px;margin-bottom:20px;font-size:.85rem;text-align:left}</style></head><body><div class=\"card\"><h3>Login</h3>%ERROR_MSG%<form method=\"POST\" action=\"/login\"><input type=\"text\" name=\"usr\" placeholder=\"Uporabniško ime\" required autocomplete=\"username\"><input type=\"password\" name=\"pswd\" placeholder=\"Geslo\" required autocomplete=\"current-password\"><button type=\"submit\">Prijava</button></form></div></body></html>";

unsigned long lastActiveSesion = 0;

void tokenTimeOut() {
  /*
    if (client_loged_out == 1) {
    client_loged_out = 0;
    DEBUG_PRINTLN("server logout.");
  }
  */
  osveziZetonSeje();
  client_loged_out = 1;
}

void osveziZetonSeje() {
  globalSessionToken = String(random(100000, 999999));
}

String loginProcessor(const String& var) {
  if (var == "ERROR_MSG") return "";
  return String();
}

String loginErrorProcessor(const String& var) {
  DEBUG_PRINT("loginError: ");
  DEBUG_PRINTLN(var);
  if (var == "ERROR_MSG") return "<div class='error'>Wrong user or pass!</div>";
  return String();
}

class SecurityFilter : public AsyncWebHandler {
public:
  SecurityFilter() {}
  virtual ~SecurityFilter() {}

  bool canHandle(AsyncWebServerRequest *request) override {
    String url = request->url();
    
    if (url == "/login") {
      return false;
    }
    
    if (url.startsWith(serverURL_ESP_CP) || 
        url.startsWith(serverURL_edit_ap) || 
        url.startsWith(serverURL_ESP_fw) || 
        url.startsWith("/edit") ||
      // url.startsWith("/wifi/") ||
      // url.startsWith("/brd") ||
      // url.startsWith("/esp/") ||
        url.startsWith(wifiPath)) {

      if (request->hasHeader("Cookie")) {
        String cookieHeader = request->header("Cookie");
        String expectedCookie = "esp_session=" + globalSessionToken;
        
        if (cookieHeader.indexOf(expectedCookie) != -1) {
          client_loged_out = 0;
          /*
          if (millis() - lastActiveSesion > interval_10m) {
            client_loged_out = 1;
            globalSessionToken = "";
            DEBUG_PRINTLN("Seja je potekla (Timeout).");
            return true;
          }
          
          lastActiveSesion = millis();
          */
          return false;
        }
      }
       //client_loged_out = 1;
      return true;
    }
    return false;
  }

  void handleRequest(AsyncWebServerRequest *request) override {
      String LOGOUT_client = request->client()->remoteIP().toString();
      DEBUG_PRINT("LOGOUT_client: ");
      DEBUG_PRINTLN(LOGOUT_client);
    //client_loged_out = 1;
    request->send_P(200, "text/html", login_html, loginProcessor);
  }

};

SecurityFilter globalSecurityHandler;

void init_global_login() {
  randomSeed(analogRead(0)); 
  osveziZetonSeje();

  server.addHandler(&globalSecurityHandler);

  server.on("/login", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", login_html, loginProcessor);
  });


  server.on("/login", HTTP_POST, [](AsyncWebServerRequest *request){
    LOGIN_client = request->client()->remoteIP().toString();

    if(request->hasArg("usr") && request->hasArg("pswd")) {
      String u = request->arg("usr");
      String p = request->arg("pswd");
      
      if(u == String(login_username) && p == String(login_password)) {
        osveziZetonSeje();
        client_loged_out = 0;
        AsyncWebServerResponse *response = request->beginResponse(303); 
        
        String cookieHeader = "esp_session=" + globalSessionToken + "; Path=/; HttpOnly; SameSite=Lax";
        response->addHeader("Set-Cookie", cookieHeader);
        
        response->addHeader("Location", serverURL_ESP_CP);
        
        request->send(response);
        return;
      }
    }
    request->send_P(401, "text/html", login_html, loginErrorProcessor);
  });


  server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request){
    client_loged_out = 1;
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "GTFO!");
    response->addHeader("Set-Cookie", "esp_session=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT; HttpOnly");
    response->addHeader("Location", "/"); 
    request->send(response);
  });
}


