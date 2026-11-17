const uint8_t save_ticker_state_time = 5;
bool updateFS_strip_state = false;
#define strip_state_path "/stripstate.json"

struct {
  int mode         = ws2812fx_mode;         // mode?
  int brt          = brightness;            // brightness
  int spd          = ws2812fx_speed;        // ws2812fx_speed
  int p_idx        = paletteIdx;            // paletteIdx
  bool isCustomPal = true;                 // "costom or builtIn"
} WS2812FXStripState;

//#define ws2812fx_mode WS2812FXStripState.mode

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
  if (ws2812_speed < SPEED_MIN)  ws2812_speed = SPEED_MIN;
  if (ws2812_speed > SPEED_MAX)  ws2812_speed = SPEED_MAX;
  return ws2812_speed;
}


bool write_strip_state_FS(){
  updateFS_strip_state = true;
  DEBUG_PRINTLN("Saving strip state FS ");

  JsonDocument jsonBuffer;
  JsonObject json = jsonBuffer.to<JsonObject>();

  json[F("mode")]     = strip ? (int) strip->getMode() : WS2812FXStripState.mode;
  json[F("brt")]      = strip ? (int) strip->getBrightness() : WS2812FXStripState.brt;
  json[F("spd")]      = WS2812FXStripState.spd;
  json[F("r")]        = main_color.red;
  json[F("g")]        = main_color.green;
  json[F("b")]        = main_color.blue;
  json[F("p_idx")]    = WS2812FXStripState.p_idx;  
  json[F("p_typ")]    = WS2812FXStripState.isCustomPal ? "c" : "b";

  if (WS2812FXStripState.isCustomPal) {
    if (WS2812FXStripState.p_idx >= 0 && WS2812FXStripState.p_idx < actual_custom_palette_count) {
      json[F("p_nm")] = myCustomPaletteNames[WS2812FXStripState.p_idx];
    } else {
      json[F("p_nm")] = F("Unknown");
    }
  } else {
    if (WS2812FXStripState.p_idx >= 0 && WS2812FXStripState.p_idx < BUILTIN_PALETTE_COUNT) {
      char nameBuffer[32];
      strcpy_P(nameBuffer, (char*)pgm_read_ptr(&(BuiltInPalettesNames[WS2812FXStripState.p_idx])));
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
  
  if (!apActive) {
    ws.textAll("Strip Saved!"); 
  }
  return true;
}


bool read_strip_state_FS() {
  updateFS_strip_state = true;
  if (LittleFS.exists(strip_state_path)) {
    DEBUG_PRINTLN("Reading stripSTATE config file... ");
    File configFile = LittleFS.open(strip_state_path, "r");
    if (configFile) {
      DEBUG_PRINT("Opened!");

      JsonDocument json_load_stripstate;
      DeserializationError error = deserializeJson(json_load_stripstate, configFile);
      DEBUG_PRINTLN("Config: ");
      if (!error) {
        DEBUG_PRINTLN("stripstate_path Parsed!");

        WS2812FXStripState.mode     = json_load_stripstate[F("mode")].as<int>();
        WS2812FXStripState.brt      = json_load_stripstate[F("brt")].as<int>();
        WS2812FXStripState.spd      = json_load_stripstate[F("spd")].as<int>();
        WS2812FXStripState.p_idx    = json_load_stripstate[F("p_idx")].as<int>();
        
        main_color.red              = json_load_stripstate[F("r")].as<int>();
        main_color.green            = json_load_stripstate[F("g")].as<int>();
        main_color.blue             = json_load_stripstate[F("b")].as<int>();
        
        String palType                 = json_load_stripstate[F("p_typ")] | "b";
        WS2812FXStripState.isCustomPal = (palType == "c");

        paletteIdx            = WS2812FXStripState.p_idx;
        ws2812fx_mode         = WS2812FXStripState.mode;
        brightness            = WS2812FXStripState.brt;
        ws2812fx_speed        = WS2812FXStripState.spd;
        
        isCustomPaletteActive = WS2812FXStripState.isCustomPal; 

        if (strip) {
          strip->stop();
          strip->strip_off();

          strip->setMode(WS2812FXStripState.mode);
          strip->setSpeed(convertSpeed(WS2812FXStripState.spd));
          strip->setBrightness(WS2812FXStripState.brt);
          
          strip->setColor(main_color.red, main_color.green, main_color.blue);

          WS2812FX::Segment* seg = strip->getSegment(0);
          if (seg != nullptr) {
            if (WS2812FXStripState.isCustomPal) {
              // Uporabniške (c) palete
              seg->colors[0] = getCustomPaletteColor(WS2812FXStripState.p_idx, 0);
              seg->colors[1] = getCustomPaletteColor(WS2812FXStripState.p_idx, 1);
              seg->colors[2] = getCustomPaletteColor(WS2812FXStripState.p_idx, 2);
            } else {
              // Vgrajene (p) palete
               if (WS2812FXStripState.p_idx >= 0 && WS2812FXStripState.p_idx < BUILTIN_PALETTE_COUNT) {
                seg->colors[0] = pgm_read_dword(&(BuiltInPalettes[WS2812FXStripState.p_idx][0]));
                seg->colors[1] = pgm_read_dword(&(BuiltInPalettes[WS2812FXStripState.p_idx][1]));
                seg->colors[2] = pgm_read_dword(&(BuiltInPalettes[WS2812FXStripState.p_idx][2]));
                
              }
            }
          }

          //strip->trigger();
          strip->start();
          strip->show();

        }

          DEBUG_PRINT("---isCustomPaletteActive: "); 
          DEBUG_PRINTLN(isCustomPaletteActive);
          DEBUG_PRINT("---paletteIdx: "); 
          DEBUG_PRINTLN(paletteIdx); 
          DEBUG_PRINT("---ws2812fx_mode: "); 
          DEBUG_PRINTLN(ws2812fx_mode); 
          DEBUG_PRINT("---WS2812FXStripState.mode: "); 
          DEBUG_PRINTLN(WS2812FXStripState.mode); 


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
