
const char* FIRMWARE_VERSION = "3.0.2";

const char* hardCode_SSID    = "SSID";
const char* hardCode_PASS    = "PWD";

const char* softAP_PASS      = "12345678";

const char* mqtt_server      = "10.10.10.10";
const int   mqtt_port        = 1883;
const char* mqtt_user        = "usr";
const char* mqtt_password    = "pwd";

const char* login_username   = "admin";
const char* login_password   = "admin";

const char* sketch_file      = "McLightning.ino";

const char* ws_path          = "/ws";

#if defined(ESP8266)
  int8_t SDA_pin     = -1;
  int8_t SCL_pin     = -1;
  int8_t Dallass_pin = -1;
#elif defined(ESP32)
  int8_t SDA_pin     = -1;
  int8_t SCL_pin     = -1;
  int8_t Dallass_pin = -1;
#endif

  //#define MDNS_DEVICE_NAME String(device_name)
  #define MDNS_SERVICE_NAME FW_BOARD_TYPE_STR
  #define MDNS_PROTOCOL "tcp"
  #define MDNS_PORT 5600

  const uint8_t www_port = 80;
  const char* device_name_c;
    char hostname[20];
  char device_name[20];


String softAPname = "";
static char chipID_buffer[20] = {0};
#if defined(ESP8266)
  void init_ChipID() {
    snprintf(chipID_buffer, sizeof(chipID_buffer), "%06x", (uint32_t)ESP.getChipId());
    softAPname = String("ESP8266_") + String(chipID_buffer);
  }
  const char* chipID = chipID_buffer;
  //String chipID_str = String(chipID_buffer);
    #define chipID_str String(ESP.getChipId(), HEX)

#elif defined(ESP32)
  void init_ChipID() {
    snprintf(chipID_buffer, sizeof(chipID_buffer), "%012llx", (uint64_t)ESP.getEfuseMac());
    softAPname = String("ESP32_") + String(chipID_buffer);
  }
  #define chipID_str String((uint32_t)ESP.getEfuseMac(), HEX)

#endif


static const char mqtt_firmware[]        = "firmware";
static const char mqtt_firmware_status[] = "firmware/status";

static const char mqtt_announce[]        = "announce";
static const char mqtt_ping[]            = "ping";

static const char mqtt_ESPinfo[]         = "espinfo";

static const char mqtt_temp[]            = "temp/periodic";
static const char mqtt_temp_state[]      = "temp/state"; 
static const char mqtt_ads[]             = "ads/periodic";
static const char mqtt_ads_state[]       = "ads/state";
static const char mqtt_shunt[]           = "shunt/periodic";
static const char mqtt_shunt_state[]     = "shunt/state";

static const char mqtt_event[]           = "event";

static const char mqtt_module[]          = "module";
static const char mqtt_system[]          = "system";
static const char mqtt_pin[]             = "pin";
static const char mqtt_state[]           = "state";

static const char mqtt_subscribe[]       = "sub";
static const char mqtt_cmd[]             = "cmd";
static const char mqtt_reboot[]          = "reboot";

//URL's

// protected sites
const char* const esp_settings_path       = "/esp.json";

const char* const serverURL_BRD_sett_html = "/brd";
const char* const serverURL_BRD_sett_json = "/brd/json";
const char* const serverURL_BRD_sett_save = "/brd/save";

const char* const serverURL_ESP_cmd       = "/esp/cmd";
const char* const serverURL_ESP_CP        = "/esp/cp"; //Control panel-ish
const char* const serverURL_ESP_info      = "/esp/info";
const char* const serverURL_ESP_fw        = "/esp/fw"; // pwd

const char* const serverURL_scan_wifi     = "/scan/wifi";
const char* const serverURL_scan_i2c      = "/scan/i2c";
const char* const serverURL_scan_OneWire  = "/scan/1w";

// Wifimanager
const char* const serverURL_edit_ap       = "/wifi/edit_ap"; // pwd
const char* const serverURL_wifi_save     = "/wifi/save_ap";

//UDP
const char* const serverURL_get_HTML      = "/udp";
const char* const serverURL_get_UDP       = "/udp/get";
const char* const serverURL_set_UDP       = "/udp/set";
const char* const serverURL_UDP_list      = "/udp/list";
const char* const serverURL_UDP_discover  = "/udp/discover";

//Pixel Path
const char* const serverURL_getModes      = "/px/modes";
const char* const serverURL_list_status   = "/px/status";
const char* const serverURL_strip_state   = "/px/conf";

const char* const serverURL_get_builtin_palettes  = "/pal/b";
const char* const serverURL_get_custom_palettes   = "/pal/c";
const char* const serverURL_save_palettes         = "/pal/save";
const char* const serverURL_edit_palettes         = "/pal/edit";

  #define notify_evt mqtt_event
  #define notify_none "B"

  #define notify_OK "OK"
  #define notify_FAILED "FAILED"
  #define notify_ERROR "ERROR"
  #define notify_RECIEVED "RECIEVED"
  #define notify_SAVED "SAVED!"
  #define notify_LCD_on "lcdon"
  #define notify_LCD_off "lcdoff"
  #define notify_LCD_dim "lcddim"
  #define notify_CONNECTED "CONNECTED"
  #define notify_DISCONNECTED "DISCONNECTED"

  #define j_idx "idx"
  #define j_name "nm"
  #define j_temp "temp"
  #define j_alarm "al"
  #define j_hysteresis "hy"
  #define j_toPin "tpin"
  #define j_mode "mo"
  #define j_blink_type "blt"
  #define j_state "st"
  #define j_pin "pin"
  #define j_duration "dur"
  #define j_value "val"
  #define j_season "seas"
  #define j_what "wh"
  #define j_counter "cnt"
  #define j_read_pin "pin_r"
  #define j_isPause "isp"
  #define j_blinkEnabled "ble"

  #define ENABLED "E"
  #define DISABLE "D"
  #define FAILED "Fail"
  #define n_ERROR "ER"
  #define n_OK "OK"
  #define NOT_FOUND "NF"

  #define ARRAY_INFO_module "module"
  #define ARRAY_INFO_sensors "info"

  #define TEMP_C "C"
  #define TEMP_F "F"
  #define HUMIDITY "hum"
  #define PRESSURE "pres"

  #define blink_OFF 0
  #define blink_ON 1
  #define blink_STANDBY 2
  #define blink_WARNING 3
  #define blink_ERROR 4
  #define blink_CRITICAL 5
  #define blink_OPEN 6
  #define blink_CLOSE 7
  #define blink_RUNNING 8
  #define blink_AP_ACTIVE 9
  #define blink_DISCONECTED 10
  #define blink_MSG 11


  //remote ota
  bool OTA_fw_update = false;
  bool OTA_fw_update_running = false;
  static char FW_file_url[256];
  bool fw_deleteFS = false;

 //Intervals
unsigned long loop_time_2 = 0, loop_time_5 = 0, loop_time_10 = 0, loop_time_30 = 0, loop_time_60 = 0, loop_time_5m = 0, loop_time_10m = 0, loop_time_1h = 0, loop_time_12h = 0;

unsigned long Seconds = 1000UL;

const unsigned long interval_2s   = 2 * Seconds; 
const unsigned long interval_5s   = 5 * Seconds;
const unsigned long interval_10s  = 10 * Seconds;
const unsigned long interval_30s  = 30 * Seconds;

const unsigned long interval_1m   = 1 * 60 * Seconds;
const unsigned long interval_5m   = 5 * 60 * Seconds;
const unsigned long interval_10m  = 10 * 60 * Seconds;
const unsigned long interval_30m  = 30 * 60 * Seconds;

const unsigned long interval_1h   = 3600UL * Seconds;
const unsigned long interval_6h   = 21600UL * Seconds;


const unsigned long QTR_SECOND   = Seconds / 4;
const unsigned long HALF_SECOND  = Seconds / 2;
const unsigned long ONE_SECOND   = 1 * Seconds;
const unsigned long TWO_SECOND   = 2 * Seconds;
const unsigned long THREE_SECOND = 3 * Seconds;
const unsigned long FOUR_SECOND  = 4 * Seconds;
const unsigned long FIVE_SECOND  = 5 * Seconds;
const unsigned long TEN_SECOND   = 10 * Seconds;