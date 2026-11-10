//Trigger saving 
void Trigger_Save_Config() {
  triggerSaveStripConfig = true; 
}

void Trigger_Save_State() {
  triggerSaveStripState = true; 
}

bool updateFS_strip_config = false;
const uint8_t save_ticker_config_time = 5;

#define strip_config_path "/stripconfig.json"

struct {
  uint16_t stripSize  = NUMLEDS;
  uint8_t RGBOrder    = NEO_GRB;
  uint8_t pin         = LED_PIN;
  uint8_t pixelType   = LED_KHZ_TYPE;
  bool udpMaster      = false;
  bool udpReciever    = false;
} WS2812FXStripSettings;


#ifdef CUSTOM_WS2812FX_ANIMATIONS
  #include "custom_ws2812fx_animations.h"
#endif

void write_strip_config_FS(void){
  updateFS_strip_config = true;
  DEBUG_PRINTLN("Saving Strip cfg: TRUE");

  JsonDocument jsonBuffer;
  JsonObject json = jsonBuffer.to<JsonObject>();

  json[F("count")]    = WS2812FXStripSettings.stripSize;
  json[F("rgb")]      = WS2812FXStripSettings.RGBOrder;
  json[F("pin")]      = WS2812FXStripSettings.pin;
  json[F("khz")]      = WS2812FXStripSettings.pixelType;
  json[F("udp_mode")] = Udp_mode_str;
  json[F("udp_m")]    = WS2812FXStripSettings.udpMaster;
  json[F("udp_r")]    = WS2812FXStripSettings.udpReciever;

  File configFile = LittleFS.open(strip_config_path, "w");
  if (!configFile) {
    DEBUG_PRINTLN("Failed to open /stripconfig.json for writing!");
    updateFS_strip_config = false;
    return;
  }
  
  serializeJson(json, configFile);

  configFile.close();
  updateFS_strip_config = false;

    ws.textAll("Strip cfg Saved"); 

    //notifyClients(notify_evt,"Strip saved");
}


bool read_strip_config_FS(void) {
  updateFS_strip_config = true;
  if (LittleFS.exists(strip_config_path)) {
    DEBUG_PRINTLN("Reading read_strip_config_FS file... ");
    File configFile = LittleFS.open(strip_config_path, "r");
    if (configFile) {
      DEBUG_PRINTLN("Opened!");

      JsonDocument json;
      DeserializationError error = deserializeJson(json, configFile);

      DEBUG_PRINTLN("stripconfig.json: ");
      if (!error) {
        DEBUG_PRINTLN("Parsed successfully!");

        WS2812FXStripSettings.stripSize     = json[F("count")]            ? json[F("count")].as<uint16_t>() : NUMLEDS;
        WS2812FXStripSettings.RGBOrder      = json[F("rgb")]              ? json[F("rgb")].as<uint8_t>() : NEO_GRB;
        WS2812FXStripSettings.pin           = json[F("pin")]              ? json[F("pin")].as<uint8_t>() : LED_PIN;
        WS2812FXStripSettings.pixelType     = json[F("khz")]              ? json[F("khz")].as<uint8_t>() : LED_KHZ_TYPE;
        WS2812FXStripSettings.udpMaster     = json[F("udp_m")].is<bool>() ? json[F("udp_m")].as<bool>() : false;
        WS2812FXStripSettings.udpReciever   = json[F("udp_r")].is<bool>() ? json[F("udp_r")].as<bool>() : false;

        configFile.close();
        updateFS_strip_config = false;
        return true;
      } else {
        DEBUG_PRINTLN("Failed to load json config!");
        configFile.close();
      }
    } else {
      DEBUG_PRINTLN("Failed to open /stripconfig.json");
    }
  } else {
    DEBUG_PRINTLN("Couldn't find stripconfig.json, using defaults.");
  }
  
  updateFS_strip_config = false;
  return false;
}

void init_strip_config_path(){
  if (!LittleFS.exists(strip_config_path)){
    write_strip_config_FS();
  } else {
    DEBUG_PRINTLN("load_strip Config!");
    read_strip_config_FS();
  }
}

void init_Strip() {

  uint16_t stripSize = WS2812FXStripSettings.stripSize;
  uint8_t RGBOrder   = WS2812FXStripSettings.RGBOrder;
  uint8_t pin        = WS2812FXStripSettings.pin;
  uint8_t pixelType  = WS2812FXStripSettings.pixelType;

  if (strip) {
    delete strip;
    strip = nullptr;
  }

  if (pixelType == 1) {
    strip = new WS2812FX(stripSize, pin, RGBOrder + NEO_KHZ400);
    DEBUG_PRINTLN("pixelType NEO_KHZ400 !");
  }
  else {
    strip = new WS2812FX(stripSize, pin, RGBOrder + NEO_KHZ800);
    DEBUG_PRINTLN("pixelType NEO_KHZ800 !");
  }

  if (strip) {
    strip->init();
    strip->setBrightness(brightness);
    strip->setSpeed(convertSpeed(ws2812fx_speed));
    strip->setColor(main_color.red, main_color.green, main_color.blue);
    
    strip->setOptions(0, GAMMA);  

    #ifdef CUSTOM_WS2812FX_ANIMATIONS
      strip->setCustomMode(0, F("Fire 2012"), myCustomEffect);
      strip->setCustomMode(1, F("Fireworks"), myCustomEffect2);
    #endif

    strip->start();
  }

  if(Mode != HOLD) Mode = SET_MODE;
  
  save_strip_config_ticker.detach();
  save_strip_config_ticker.once(save_ticker_config_time, Trigger_Save_Config);
}
