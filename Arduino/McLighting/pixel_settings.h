#define LED_PIN 5
#define NUMLEDS 50
#define LED_KHZ_TYPE 1   // 1 = 800kHz (WS2812B), 2 = 400kHz (WS2811)

#define ENABLE_LEGACY_ANIMATIONS
#define CUSTOM_WS2812FX_ANIMATIONS

bool reverseMode  = false;
bool skipFirstLed = false;   

uint32_t autoParams[4][4] = {
  {0xff0000, 200,  1,  5000},
  {0x00ff00, 200,  3, 10000},
  {0x0000ff, 200, 14,  5000},
  {0x0000ff, 200, 45, 15000}
};

#ifdef ENABLE_LEGACY_ANIMATIONS
  enum MODE { SET_MODE, HOLD, OFF, SETCOLOR, SETSPEED, BRIGHTNESS, WIPE, RAINBOW, RAINBOWCYCLE, THEATERCHASE, TWINKLERANDOM, THEATERCHASERAINBOW, TV, CUSTOM };
  MODE Mode = RAINBOW;
  bool exit_func = false;
#else
  enum MODE { SET_MODE, HOLD, OFF, SETCOLOR, SETSPEED, BRIGHTNESS, CUSTOM };
  MODE Mode = SET_MODE;        
#endif

MODE prevmode = Mode;

int ws2812fx_speed = 196;
int brightness = 196;
int ws2812fx_mode = 0;

struct ledstate
{
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

typedef struct ledstate LEDState;
LEDState ledstates[NUMLEDS];
LEDState main_color = { 255, 0, 0 };
