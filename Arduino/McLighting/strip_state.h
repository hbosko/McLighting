const uint8_t save_ticker_state_time = 5;

bool updateFS_strip_state = false;
#define strip_state_path "/stripstate.json"

void init_strip_state_path(){
  if (!LittleFS.exists(strip_state_path)){
    write_strip_state_FS();
  } else {
    DEBUG_PRINTLN("load_Default_strip State!");
    read_strip_state_FS();
  }
}

uint16_t convertSpeed(uint8_t mcl_speed) {
  uint16_t ws2812_speed = 61760 * (exp(0.0002336 * mcl_speed) - exp(-0.03181 * mcl_speed));
  ws2812_speed = SPEED_MAX - ws2812_speed;
  if (ws2812_speed < SPEED_MIN) {
    ws2812_speed = SPEED_MIN;
  }
  if (ws2812_speed > SPEED_MAX) {
    ws2812_speed = SPEED_MAX;
  }
  return ws2812_speed;
}

bool write_strip_state_FS(){
  updateFS_strip_state = true;
  DEBUG_PRINTLN("Saving strip state FS ");

  JsonDocument jsonBuffer;
  JsonObject json = jsonBuffer.to<JsonObject>();

  json[F("mode")]     = strip ? (int) strip->getMode() : (int) ws2812fx_mode;
  json[F("brt")]      = brightness;
  json[F("spd")]      = ws2812fx_speed;
  json[F("r")]        = main_color.red;
  json[F("g")]        = main_color.green;
  json[F("b")]        = main_color.blue;
  json[F("p_idx")]    = paletteIdx;  
  json[F("p_typ")]    = isCustomPaletteActive ? "c" : "b";

  if (isCustomPaletteActive) {
    if (paletteIdx >= 0 && paletteIdx < actual_custom_palette_count) {
      json[F("p_nm")] = myCustomPaletteNames[paletteIdx];
    } else {
      json[F("p_nm")] = F("Unknown");
    }
  } else {
    if (paletteIdx >= 0 && paletteIdx < BUILTIN_PALETTE_COUNT) {
      char nameBuffer[32];
      strcpy_P(nameBuffer, (char*)pgm_read_ptr(&(BuiltInPalettesNames[paletteIdx])));
      json[F("p_nm")] = nameBuffer;
    } else {
      json[F("p_nm")] = F("Unknown");
    }
  }

  File configFile = LittleFS.open(strip_state_path, "w");
  if (!configFile) {
    DEBUG_PRINTLN("Failed to open /stripstate.json for writing!");
    updateFS_strip_state = false;
    return false;
  }

  serializeJson(json, configFile);
  configFile.close();
  
  updateFS_strip_state = false;
  ws.textAll("Strip Saved!"); 
  return true;
}


bool read_strip_state_FS() {
  updateFS_strip_state = true;
  if (LittleFS.exists(strip_state_path)) {
    DEBUG_PRINTLN("Reading stripstate_path config file... ");
    File configFile = LittleFS.open(strip_state_path, "r");
    if (configFile) {
      DEBUG_PRINT("Opened!");

      JsonDocument json_load_stripstate;
      DeserializationError error = deserializeJson(json_load_stripstate, configFile);
      DEBUG_PRINTLN("Config: ");
      if (!error) {
        DEBUG_PRINTLN("stripstate_path Parsed!");

        ws2812fx_mode     = json_load_stripstate[F("mode")].as<int>();
        brightness        = json_load_stripstate[F("brt")].as<int>();
        ws2812fx_speed    = json_load_stripstate[F("spd")].as<int>();
        main_color.red    = json_load_stripstate[F("r")].as<int>();
        main_color.green  = json_load_stripstate[F("g")].as<int>();
        main_color.blue   = json_load_stripstate[F("b")].as<int>();
        
        paletteIdx        = json_load_stripstate[F("p_idx")].as<int>();

        String palType    = json_load_stripstate[F("p_typ")] | "b";
        isCustomPaletteActive = (palType == "c");

        if (strip) {
          strip->setMode(ws2812fx_mode);
          strip->setSpeed(convertSpeed(ws2812fx_speed));
          strip->setBrightness(brightness);
          
          WS2812FX::Segment* seg = strip->getSegment(0);
          if (seg != nullptr) {
            if (isCustomPaletteActive) {
              seg->colors[0] = getCustomPaletteColor(paletteIdx, 0);
              seg->colors[1] = getCustomPaletteColor(paletteIdx, 1);
              seg->colors[2] = getCustomPaletteColor(paletteIdx, 2);
            } else {
              if (paletteIdx >= 0 && paletteIdx < BUILTIN_PALETTE_COUNT) {
                seg->colors[0] = pgm_read_dword(&(BuiltInPalettes[paletteIdx][0]));
                seg->colors[1] = pgm_read_dword(&(BuiltInPalettes[paletteIdx][1]));
                seg->colors[2] = pgm_read_dword(&(BuiltInPalettes[paletteIdx][2]));
              }
            }
          }

          strip->setColor(main_color.red, main_color.green, main_color.blue);
        }

        configFile.close();
        updateFS_strip_state = false;
        return true;
      } else {
        DEBUG_PRINTLN("Failed to load stripstate_path json config!");
        configFile.close();
      }
    } else {
      DEBUG_PRINTLN("Failed to open /stripstate_path.json");
    }
  } else {
    DEBUG_PRINTLN("Could not find stripstate_path.json");
  }
  updateFS_strip_state = false;
  return false;
}

void listModesJSON(Print& output) {
  JsonDocument doc;
  JsonArray root = doc.to<JsonArray>();

  if (strip) {
    for (uint8_t i = 0; i < strip->getModeCount(); i++) {
      JsonObject object = root.add<JsonObject>();
      object[F("id")]   = i;
      object[F("nm")]   = strip->getModeName(i);
    }
  }
  serializeJson(doc, output);
}
