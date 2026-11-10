
const IPAddress localIP(4, 4, 4, 1);
const IPAddress gatewayIP(4, 4, 4, 1);
const IPAddress subnetMask(255, 255, 255, 0);

const String localIPURL = "http://4.4.4.1";

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

  // const char* give me sometimes weird characters & failing to (re)connect, going back to String (works fine)
String hostNameParam = "hostname";
String ssidParam = "ssid";
String passParam = "pass";

String ssid;
String pass;

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
  String WMssid;
  String WMpass;
    #ifdef ENABLE_MQTT
      String WMmqttServer;
      uint16_t WMmqttPort;
      String WMmqttUser;
      String WMmqttPass;
    #endif
};

WifiManagerConfig WMconfig;

void initWMConfigDefaults() {
  WMconfig.WMhostName = device_name_c;
  WMconfig.WMport = www_port;
  WMconfig.WMssid = ssid;
  WMconfig.WMpass = pass;
  
  #ifdef ENABLE_MQTT
    WMconfig.WMmqttServer = String(mqttServer);
    WMconfig.WMmqttPort   = mqttPort;
    WMconfig.WMmqttUser   = String(mqttUser);
    WMconfig.WMmqttPass   = String(mqttPass);
  #endif
}