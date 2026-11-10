
const char* custom_types[] = {
  "PWM", 
  "DUMB",
  "RGB",
  "RGBW"
};

const char* custom_sub_types[] = {
  "Mega_Tree", 
  "Mini_Tree", 
  "Zvezda", 
  "Ograja", 
  "Okno", 
  "Streha", 
  "Balkon", 
  "Štraf",
  "Jaslice",
  "Ostalo"
};

const char* default_type = "SMART_LED";
const char* default_subtype = "smart_led";

const size_t CUSTOM_TYPES_size = sizeof(custom_types) / sizeof(custom_types[0]);
const size_t CUSTOM_SUB_TYPES_size = sizeof(custom_sub_types) / sizeof(custom_sub_types[0]);

struct ModuleDefinition {
  uint8_t id;
  const char* type;
  const char* sub_type;
};

const ModuleDefinition SEZNAM_MODULOV[] = {
  {0,  default_type, default_subtype}, // leave
  {99, "CUSTOM",   "CUSTOM"}    // alone!
};
const size_t STEVILO_MODULOV = sizeof(SEZNAM_MODULOV) / sizeof(SEZNAM_MODULOV[0]);

const struct ModuleDefinition* poisci_modul(uint8_t iskana_id) {
  for (size_t i = 0; i < STEVILO_MODULOV; i++) {
    if (SEZNAM_MODULOV[i].id == iskana_id) {
      return &SEZNAM_MODULOV[i];
    }
  }
  return nullptr; 
}

const char* MODULE_TYPE(uint8_t id) {
  if (id == 0) {
    return default_type;
  }
  if (id == 99) {
    return custom_type;
  }
  
  const struct ModuleDefinition* m = poisci_modul(id);
  if (m != nullptr) {
    return m->type;
  }
  
  if (id >= 1 && id <= CUSTOM_TYPES_size) {
    return custom_types[id - 1];
  }
  
  return "Unknown";
}

const char* MODULE_sub_TYPE(uint8_t id) {
  if (id == 0) {
    return default_subtype;
  }
  if (id == 99) {
    return custom_sub_type;
  }
  
  const struct ModuleDefinition* m = poisci_modul(id);
  if (m != nullptr) {
    return m->sub_type;
  }
  
  if (id >= 1 && id <= CUSTOM_SUB_TYPES_size) {
    return custom_sub_types[id - 1];
  }
  
  return "Unknown";
}



void brd_json(Print& output) {
  JsonDocument doc;
  
  JsonObject doc_type = doc[F("TYPE")].to<JsonObject>();
  JsonObject doc_subtype = doc[F("SUB_TYPE")].to<JsonObject>();

  for (size_t i = 0; i < STEVILO_MODULOV; i++) {
    char key[4]; 
    itoa(SEZNAM_MODULOV[i].id, key, 10);

    doc_type[key]    = SEZNAM_MODULOV[i].type;
    doc_subtype[key] = SEZNAM_MODULOV[i].sub_type;
  }

  for (size_t i = 0; i < CUSTOM_TYPES_size; i++) {
    char key[4];
    itoa(i + 1, key, 10);
    doc_type[key] = custom_types[i];
  }

  for (size_t i = 0; i < CUSTOM_SUB_TYPES_size; i++) {
    char key[4];
    itoa(i + 1, key, 10);
    doc_subtype[key] = custom_sub_types[i];
  }

  serializeJson(doc, output);
}


void init_MODULE_pins() {
  DEBUG_PRINTLN("Inicializacija strojne opreme in pinov...");

  if (SDA_pin >= 0 && SCL_pin >= 0) {
    MODULE_I2C = 1;
    DEBUG_PRINTF("I2C vodilo -> SDA: %d, SCL: %d\r\n", SDA_pin, SCL_pin);
    Wire.begin(SDA_pin, SCL_pin);
  } else {
    MODULE_I2C = 0;
    DEBUG_PRINTLN("I2C onemogočen (SDA ali SCL je nastavljen na -1).");
  }

  // 4. DALLAS SENZOR
  if (Dallass_pin >= 0) {
    MODULE_OneWire = 1;
    DEBUG_PRINTF("Dallas senzor na pinu: %d\r\n", Dallass_pin);
    
    oneWire.begin(Dallass_pin); 
    sensors.begin();
    
    DEBUG_PRINTLN("Dallas senzor uspešno zagnan.");
  } else {
    MODULE_OneWire = 0;
    DEBUG_PRINTLN("Dallas senzor je onemogočen.");
  }

}


void init_esp_settings() {
  if (!LittleFS.exists(esp_settings_path)) {
    write_esp_settings(esp_settings_path);
  } else {
    read_esp_settings(esp_settings_path);
  }
    init_MODULE_pins();
}

void handle_esp_settings_save(AsyncWebServerRequest* request) {
  int params = request->params();
  bool dynamic_redirect = false;

  for (int i = 0; i < params; i++) {
    const AsyncWebParameter* p = request->getParam(i);
    
    DEBUG_PRINTF("Param name: %s | value: %s\r\n", p->name().c_str(), p->value().c_str());
  
    if (p->name() == "save") {
      JsonDocument json;

      DeserializationError error = deserializeJson(json, p->value());
      if (!error) {
        DEBUG_PRINTLN("Parsed successfully!");

        conf_type     = json[F("type")][F("type")] | conf_type;
        conf_sub_type = json[F("type")][F("sub_type")] | conf_sub_type;

        const char* c_type_str = json[F("custom")][F("c_type")] | nullptr;
        if (c_type_str != nullptr) {
          strlcpy(custom_type, c_type_str, sizeof(custom_type));
        }

        const char* c_sub_type_str = json[F("custom")][F("c_sub_type")] | nullptr;
        if (c_sub_type_str != nullptr) {
          strlcpy(custom_sub_type, c_sub_type_str, sizeof(custom_sub_type));
        }

        JsonObject pinz = json[F("pin")];
        if (!pinz.isNull()) {
          Dallass_pin = pinz[F("dallas")] | Dallass_pin;
          SDA_pin     = pinz[F("sda")]    | SDA_pin;
          SCL_pin     = pinz[F("scl")]    | SCL_pin;
          //MOSI_pin    = pinz[F("mosi")]   | MOSI_pin;
          //MISO_pin    = pinz[F("miso")]   | MISO_pin;
          //SS_pin      = pinz[F("ss")]     | SS_pin;
          //SCK_pin     = pinz[F("sck")]    | SCK_pin;
          //EN_pin      = pinz[F("en")]     | EN_pin;
          //DC_pin      = pinz[F("dc")]     | DC_pin;
          //buzzer_pin  = pinz[F("buzzer")] | buzzer_pin;
        }

        write_esp_settings(esp_settings_path);
        
        notifyClients(notify_evt, "esp settings saved!");
        dynamic_redirect = true;
        //esp_reboot = true;
      } else {
        DEBUG_PRINTF("JSON Deserialization Error: %s\r\n", error.c_str());
      }
    }

    if (p->name() == "default") {
      if (p->value() == "yes") {
        DEBUG_PRINTLN("DeleteFile(esp_settings_path)");
        LittleFS.remove(esp_settings_path);
        
        dynamic_redirect = true;
        esp_reboot = true;
      } else {
        DEBUG_PRINTLN("DeleteFile(esp_settings_path) canceled");
      }
    }
  }

  if (dynamic_redirect) {
    request->redirect("/");
  } else {
    request->send(400, "text/plain", "Bad Request");
  }
}


void write_esp_settings(const char* path) {
  File file = LittleFS.open(path, "w");
  if (!file) {
    DEBUG_PRINTLN("Failed to open file for writing");
    return;
  }
  serialize_esp_settings(file);
  file.close();
  DEBUG_PRINTLN("File written and closed");

    notifyClients(notify_evt, "brd settings saved!");
    esp_reboot = true;
}


void serialize_esp_settings(Print& output) {
  JsonDocument doc;

  doc[F("type")][F("type")]     = conf_type;
  doc[F("type")][F("sub_type")] = conf_sub_type;

  doc[F("custom")][F("c_type")]     = custom_type;
  doc[F("custom")][F("c_sub_type")] = custom_sub_type;

  JsonObject pinz = doc[F("pin")].to<JsonObject>();
  pinz[F("dallas")] = Dallass_pin;
  pinz[F("scl")]    = SCL_pin;
  pinz[F("sda")]    = SDA_pin;
  //pinz[F("mosi")]   = MOSI_pin;
  //pinz[F("miso")]   = MISO_pin;
  //pinz[F("ss")]     = SS_pin;
  //pinz[F("sck")]    = SCK_pin;
  //pinz[F("en")]     = EN_pin;
  //pinz[F("dc")]     = DC_pin;
  //pinz[F("buzzer")] = buzzer_pin;

  serializeJson(doc, output);
}


void read_esp_settings(const char* path) {
  if (!LittleFS.exists(path)) {
    DEBUG_PRINTLN("config file does not exist, using defaults.");
    return;
  }

  File file = LittleFS.open(path, "r");
  if (!file) {
    DEBUG_PRINTLN("Failed to open file for reading");
    return;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    DEBUG_PRINTF("Failed to parse config JSON: %s\r\n", error.c_str());
    return;
  }

  DEBUG_PRINTLN("config loaded and parsed successfully.");

  conf_type     = doc[F("type")][F("type")] | conf_type;
  conf_sub_type = doc[F("type")][F("sub_type")] | conf_sub_type;

  const char* c_type_str = doc[F("custom")][F("c_type")] | nullptr;
  if (c_type_str != nullptr) {
    strlcpy(custom_type, c_type_str, sizeof(custom_type));
  }

  const char* c_sub_type_str = doc[F("custom")][F("c_sub_type")] | nullptr;
  if (c_sub_type_str != nullptr) {
    strlcpy(custom_sub_type, c_sub_type_str, sizeof(custom_sub_type));
  }

  JsonObject pinz = doc[F("pin")];
  if (!pinz.isNull()) {
    Dallass_pin = pinz[F("dallas")] | Dallass_pin;
    SCL_pin     = pinz[F("scl")]    | SCL_pin;
    SDA_pin     = pinz[F("sda")]    | SDA_pin;
  //  MOSI_pin    = pinz[F("mosi")]   | MOSI_pin;
  //  MISO_pin    = pinz[F("miso")]   | MISO_pin;
  //  SS_pin      = pinz[F("ss")]     | SS_pin;
  //  SCK_pin     = pinz[F("sck")]    | SCK_pin;
  //  EN_pin      = pinz[F("en")]     | EN_pin;
  //  DC_pin      = pinz[F("dc")]     | DC_pin;
  //  buzzer_pin  = pinz[F("buzzer")] | buzzer_pin; 
  }
}
