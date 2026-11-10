
void server_builtin_palettes(){
  server.on(serverURL_get_builtin_palettes, HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    palettes_builtin_json(*response);
    request->send(response);
  });
}

void palettes_builtin_json(Print& output) {
  JsonDocument doc;
  
  doc[F("type")] = "palettes";
  JsonArray list = doc["list"].to<JsonArray>();

  for (uint8_t i = 0; i < BUILTIN_PALETTE_COUNT; i++) {
    JsonObject paletteObj = list.add<JsonObject>();
    paletteObj[F("id")] = i;
    
    char nameBuffer[32];
    strcpy_P(nameBuffer, (char*)pgm_read_ptr(&(BuiltInPalettesNames[i])));
    paletteObj[F("nm")] = nameBuffer;

    JsonArray colorsArray = paletteObj["clr"].to<JsonArray>();
    
    for (uint8_t c = 0; c < 3; c++) {
      char hexBuffer[9]; 
      
      uint32_t color = pgm_read_dword(&(BuiltInPalettes[i][c]));
      
      sprintf(hexBuffer, "#%06X", color); 
      colorsArray.add(hexBuffer);
    }
  }

  serializeJson(doc, output);
}