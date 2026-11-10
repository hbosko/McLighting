const unsigned int udpPort = 2390;

bool isUdpMaster = false;
bool isUdpReciever  = false;

unsigned long lastUdpSendTime = 0;

struct DiscoveredDevice {
  unsigned long lastSeen;
  char ip[16];
  char type[64];
  char mode[10];
};

#define MAX_DISCOVERED_DEVICES 10
DiscoveredDevice discoveredDevices[MAX_DISCOVERED_DEVICES];

int discoveredDeviceCount = 0;
const char* Udp_mode_str = "standalone";