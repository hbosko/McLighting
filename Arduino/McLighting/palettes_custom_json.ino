#define palette_custom_path "/palette.json"

void save_palette_custom_json() {
  JsonDocument doc;
  
  doc[F("type")] = "palettes";
  JsonArray list = doc["list"].to<JsonArray>();

  for (uint8_t i = 0; i < actual_custom_palette_count; i++) {
    JsonObject paletteObj = list.add<JsonObject>();
    paletteObj[F("id")] = i;
    paletteObj[F("name")] = myCustomPaletteNames[i];

    JsonArray colorsArray = paletteObj["colors"].to<JsonArray>();
    for (uint8_t c = 0; c < 3; c++) {
      char hexBuffer[9]; 
      sprintf(hexBuffer, "#%06X", myCustomPalettes[i][c]);
      colorsArray.add(hexBuffer);
    }
  }

  File configFile = LittleFS.open(palette_custom_path, "w");
  if (configFile) {
      DEBUG_PRINTLN("Shranjujem palete na Flash ...");
    serializeJson(doc, configFile);
    configFile.close();
  }
}

void load_palette_custom_json() {
  File configFile = LittleFS.open(palette_custom_path, "r");
  if (!configFile) {
    actual_custom_palette_count = 0;
    return;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, configFile);
  configFile.close();

  if (error) {
      DEBUG_PRINTLN("error read JSON palettes!");
    actual_custom_palette_count = 0;
    return;
  }

  JsonArray list = doc["list"].as<JsonArray>();
  actual_custom_palette_count = 0;

  for (JsonObject palette : list) {
    if (actual_custom_palette_count >= MAX_CUSTOM_PALETTES) break;

    uint8_t idx = actual_custom_palette_count;
    
    const char* name = palette["name"] | "unknown palette";
    strncpy(myCustomPaletteNames[idx], name, sizeof(myCustomPaletteNames[idx]) - 1);
    myCustomPaletteNames[idx][sizeof(myCustomPaletteNames[idx]) - 1] = '\0';

    JsonArray colors = palette["colors"].as<JsonArray>();
    for (uint8_t c = 0; c < 3 && c < colors.size(); c++) {
      const char* hexColor = colors[c];
      if (hexColor && hexColor[0] == '#') {
        myCustomPalettes[idx][c] = strtoul(hexColor + 1, NULL, 16);
      } else {
        myCustomPalettes[idx][c] = 0x000000;
      }
    }
    actual_custom_palette_count++;
  }

    DEBUG_PRINT("Load palettes from FS: ");
    DEBUG_PRINTLN(actual_custom_palette_count);
}

void init_palette_custom_json(){
  if (!LittleFS.exists(palette_custom_path)){
      DEBUG_PRINTLN("Palette.json not found, generating new...");
    actual_custom_palette_count = 0;
    save_palette_custom_json();
  } else {
    load_palette_custom_json();
  }
}

void handler_save_palettes(){
  server.addHandler(new AsyncCallbackJsonWebHandler(serverURL_save_palettes, [](AsyncWebServerRequest *request, JsonVariant &json) {
    
    if (!json.is<JsonObject>()) {
      request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Wrong data format\"}");
      return;
    }

    JsonObject doc = json.as<JsonObject>();

    // inject protection
    if (doc["type"].isNull() || doc["type"] != "palettes" || !doc["list"].is<JsonArray>()) {
        DEBUG_PRINTLN("neveljavne JSON strukture zavrnjen!");
      request->send(422, "application/json", "{\"status\":\"error\",\"message\":\"Denied: Wrong svructure palette!\"}");
      return;
    }

    File configFile = LittleFS.open(palette_custom_path, "w");
    if (!configFile) {
      request->send(500, "application/json", "{\"status\":\"error\",\"message\":\"LittleFS err open file palette.json\"}");
      return;
    }

    serializeJson(doc, configFile);
    configFile.close();

    load_palette_custom_json(); 

      DEBUG_PRINTLN("Palete,json read OK!");

    request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Palete success saved!\"}");
  }, 4000)); // JSON limit 4KB
}

