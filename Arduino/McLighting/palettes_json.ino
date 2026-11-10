
const uint8_t gGradientPaletteCount = 3;

const char* const gGradientPaletteNames[] PROGMEM = {
  "Sunset Real", "Rivendell", "Ocean Breeze", "Red & Blue", "Yellowout", 
  "Analogous", "Splash", "Pastel", "Sunset 2", "Beech", "Vintage", 
  "Departure", "Landscape", "Beach", "Sherbet", "Hult", "Hult 64", 
  "Drywet", "Jul", "Grintage", "Rewhi", "Tertiary", "Fire", "Icefire", 
  "Cyane", "Light Pink", "Autumn", "Magenta", "Magred", "Yelmag", 
  "Yelblu", "Orange & Teal", "Tiamat", "April Night", "Orangery", "C9", 
  "Sakura", "Aurora", "Atlantica", "C9 2", "C9 New", "Temperature", 
  "Aurora 2", "Retro Clown", "Candy", "Toxy Reaf", "Fairy Reaf", 
  "Semi Blue", "Pink Candy", "Red Reaf", "Aqua Flash", "Yelblu Hot", 
  "Lite Light", "Red Flash", "Blink Red", "Red Shift", "Red Tide", "Candy 2"
};


void palette_json(Print& output) {
  JsonDocument doc;
  
  doc["type"] = "palettes";
  JsonArray list = doc["list"].to<JsonArray>();

  for (uint8_t i = 0; i < gGradientPaletteCount; i++) {
    JsonObject paletteObj = list.add<JsonObject>();
    paletteObj["id"] = i;
    
    char nameBuffer[32];
    strcpy_P(nameBuffer, (char*)pgm_read_ptr(&(gGradientPaletteNames[i])));
    paletteObj["name"] = nameBuffer;
  }

  //String output;
  serializeJson(doc, output);

  #ifdef DEBUG
  Serial.println(F("JSON seznam palet uspešno poslan na Web UI."));
  #endif
}
