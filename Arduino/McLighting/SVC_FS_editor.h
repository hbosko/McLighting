#ifdef ENABLE_FS_Editor

void init_FS_editor() {
  server.on("/fs-status", HTTP_GET, [](AsyncWebServerRequest *request) {
    /*
    if(String(FS_username).length() && String(FS_password).length() && !request->authenticate(FS_username, FS_password)) {
      return request->requestAuthentication();
    }
    */
    size_t totalBytes = 0;
    size_t usedBytes = 0;

    #ifdef ESP32
      totalBytes = LittleFS.totalBytes();
      usedBytes = LittleFS.usedBytes();
    #elif defined(ESP8266)
      FSInfo fs_info;
      LittleFS.info(fs_info);
      totalBytes = fs_info.totalBytes;
      usedBytes = fs_info.usedBytes;
    #endif

    String json = "{\"total\":" + String(totalBytes) + ",\"used\":" + String(usedBytes) + "}";
    request->send(200, "application/json", json);
  });

  /*
    server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncWebServerResponse *response = request->beginResponse(401, "text/plain", "Odjavljeni ste.");
      response->addHeader("WWW-Authenticate", "Basic realm=\"ESP Login\"");
      request->send(response);
    });
  */

    #ifdef PASSWD_PROTECTED
      #ifdef ESP32
        SPIFFSEditor* editor = new SPIFFSEditor(LittleFS, login_username, login_password);
      #elif defined(ESP8266)
        SPIFFSEditor* editor = new SPIFFSEditor(login_username, login_password, LittleFS);
      #endif
    #else
      #ifdef ESP32
        SPIFFSEditor* editor = new SPIFFSEditor(LittleFS, "", "");
      #elif defined(ESP8266)
        SPIFFSEditor* editor = new SPIFFSEditor("", "", LittleFS);
      #endif
    #endif


  server.addHandler(editor);
}
#endif