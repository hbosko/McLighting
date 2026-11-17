
const IPAddress localIP(4, 4, 4, 1);
const IPAddress gatewayIP(4, 4, 4, 1);
const IPAddress subnetMask(255, 255, 255, 0);

const char* localIPURL = "http://4.4.4.1";

#define MAX_CLIENTS 3
#define WIFI_CHANNEL 11

#if defined(ESP8266)
  WiFiEventHandler wifiConnectHandler;
  WiFiEventHandler wifiDisconnectHandler;
#elif defined(ESP32)

#endif

int wm_led_count = 0;

unsigned long WM_loop = 0;
unsigned long lastTime = 0;
uint8_t AP_count = 0;

bool apActive = false;
bool firstBoot = false;
bool manual_reboot_req = false;
bool manual_reboot_req_last = false;

// html params
const char* mqqtPasswdParam = "mqpass";
const char* mqqtUserParam = "mquser";
const char* mqqtServerParam = "mqsrv";
const char* mqqtPortParam = "mqport";

unsigned long wifiMillis = 0;
const unsigned long WIFI_TIMEOUT = 10000; // 10 sekund 
bool wifi_connecting = false;

//AP try to reconnect
unsigned long lastTryInAp = 0;
const unsigned long RECONNECT_INTERVAL = interval_1m;

const char* hostNameParam = "hostname";
const char* ssidParam = "ssid";
const char* passParam = "pass";

char ssid[33] = "";
char pass[65] = "";


#ifdef ENABLE_MQTT
  const char* mqttServer = mqtt_server;
  uint16_t mqttPort = mqtt_port;
  const char* mqttUser = mqtt_user;
  const char* mqttPass = mqtt_password;
#endif

#define wifiPath "/wifi.json"

struct WifiManagerConfig {
  const char* WMhostName;
  uint8_t WMport;
  char WMssid[33];
  char WMpass[65];
    #ifdef ENABLE_MQTT
      char WMmqttServer[65];
      uint16_t WMmqttPort;
      char WMmqttUser[33];
      char WMmqttPass[64];
    #endif
};

WifiManagerConfig WMconfig;

void initWMConfigDefaults() {
  WMconfig.WMhostName = device_name;
  WMconfig.WMport     = www_port;
  
  snprintf(WMconfig.WMssid, sizeof(WMconfig.WMssid), "%s", ssid);
  snprintf(WMconfig.WMpass, sizeof(WMconfig.WMpass), "%s", pass);
  
  #ifdef ENABLE_MQTT
    snprintf(WMconfig.WMmqttServer, sizeof(WMconfig.WMmqttServer), "%s", mqttServer);
    WMconfig.WMmqttPort = mqttPort;
    snprintf(WMconfig.WMmqttUser, sizeof(WMconfig.WMmqttUser), "%s", mqttUser);
    snprintf(WMconfig.WMmqttPass, sizeof(WMconfig.WMmqttPass), "%s", mqttPass);
  #endif
}
