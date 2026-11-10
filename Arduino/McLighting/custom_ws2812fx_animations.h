//******Palette Animation Globals*******************************************
uint8_t targetpicker;
uint8_t colorIndex;
String anim_direction = "forward";
CRGBPalette16 currentPalette;
CRGBPalette16 targetPalette;
TBlendType    currentBlending;

#define RGBW32(r,g,b,w) (uint32_t((byte(w) << 24) | (byte(r) << 16) | (byte(g) << 8) | (byte(b))))

#define COOLING  55
#define SPARKING 120
bool gReverseDirection = false;

void Fire2012()
{
  static byte heat[NUMLEDS];

  for(int i = 0; i < WS2812FXStripSettings.stripSize; i++) {
    heat[i] = qsub8(heat[i], random8(0, ((COOLING * 10) / WS2812FXStripSettings.stripSize) + 2));
  }

  for(int k = WS2812FXStripSettings.stripSize - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }

  if(random8() < SPARKING) {
    int y = random8(7);
    heat[y] = qadd8(heat[y], random8(160, 255));
  }

  for(int j = 0; j < WS2812FXStripSettings.stripSize; j++) {
    CRGB color = HeatColor(heat[j]);
    int pixelnumber = gReverseDirection ? (WS2812FXStripSettings.stripSize - 1) - j : j;
    if(strip) strip->setPixelColor(pixelnumber, color.red, color.green, color.blue);
  }
}

uint16_t myCustomEffect() {
  Fire2012();
  return (strip->getSpeed() / WS2812FXStripSettings.stripSize);
}


#define NUM_SPARKS 62 
float sparkPos[NUM_SPARKS];
float sparkVel[NUM_SPARKS];
float sparkCol[NUM_SPARKS];
float flarePos;
float flareVel;
float flareBrightness;
float dying_gravity;
int nSparks = 0;
float gravity = -.004;

enum FireworkState { LAUNCH_INIT, FLYING, EXPLOSION_INIT, EXPLODING, BREAK_BETWEEN };
FireworkState fireworkStep = LAUNCH_INIT;
unsigned long breakStartTime = 0;
unsigned long breakDuration = 0;

void runFlareFrame() {
  FastLED.clear();

  for (int i = 0; i < 5; i++) {
    sparkPos[i] += sparkVel[i];
    sparkPos[i] = constrain(sparkPos[i], 0, 120);
    sparkVel[i] += gravity;
    sparkCol[i] += -.8;
    sparkCol[i] = constrain(sparkCol[i], 0, 255);
    // Opomba: če želite risati izstrelitvene iskre, odkomentirajte spodaj:
     CRGB sCol = HeatColor(sparkCol[i]);
     if(strip) strip->setPixelColor(int(sparkPos[i]), sCol.r, sCol.g, sCol.b);
  }
  
  if(strip) strip->setPixelColor(int(flarePos), 0, 0, int(flareBrightness * 255));
  
  flarePos += flareVel;
  flareVel += gravity;
  flareBrightness *= .985;
}

void runExplodeFrame() {
  FastLED.clear();
  float c1 = 120; 
  float c2 = 50; 

  for (int i = 0; i < nSparks; i++) { 
    sparkPos[i] += sparkVel[i]; 
    sparkPos[i] = constrain(sparkPos[i], 0, WS2812FXStripSettings.stripSize - 1); 
    sparkVel[i] += dying_gravity; 
    sparkCol[i] *= .99; 
    sparkCol[i] = constrain(sparkCol[i], 0, 255); 
    
    if(sparkCol[i] > c1) { 
      if(strip) strip->setPixelColor(int(sparkPos[i]), 255, 255, (255 * (sparkCol[i] - c1)) / (255 - c1));
    }
    else if (sparkCol[i] < c2) {
      if(strip) strip->setPixelColor(int(sparkPos[i]), (255 * sparkCol[i]) / c2, 0, 0);
    }
    else {
      if(strip) strip->setPixelColor(int(sparkPos[i]), 255, (255 * (sparkCol[i] - c2)) / (c1 - c2), 0);
    }
  }
  dying_gravity *= .995;
}

uint16_t myCustomEffect2() {
  
  switch (fireworkStep) {
    
    case LAUNCH_INIT:
      flarePos = 0;
      flareVel = float(random16(50, 90)) / 100.0; 
      flareBrightness = 1.0;

      for (int i = 0; i < 5; i++) { 
        sparkPos[i] = 0;
        sparkVel[i] = (float(random8()) / 255.0) * (flareVel / 5.0);
        sparkCol[i] = sparkVel[i] * 1000.0;
        sparkCol[i] = constrain(sparkCol[i], 0, 255);
      } 
      fireworkStep = FLYING;
      break;

    case FLYING: 
      if (flareVel >= -.2) {
        runFlareFrame();
      } else {
        fireworkStep = EXPLOSION_INIT; 
      }
      break;

    case EXPLOSION_INIT: 
      nSparks = flarePos / 2;
      if (nSparks > NUM_SPARKS) nSparks = NUM_SPARKS; 
      
      for (int i = 0; i < nSparks; i++) { 
        sparkPos[i] = flarePos; 
        sparkVel[i] = (float(random16(0, 20000)) / 10000.0) - 1.0; 
        sparkCol[i] = abs(sparkVel[i]) * 500.0; 
        sparkCol[i] = constrain(sparkCol[i], 0, 255); 
        sparkVel[i] *= flarePos / WS2812FXStripSettings.stripSize; 
      } 
      sparkCol[0] = 255;
      dying_gravity = gravity;
      fireworkStep = EXPLODING;
      break;

    case EXPLODING:
      if (sparkCol[0] > (50.0 / 128.0)) {
        runExplodeFrame();
      } else {
        FastLED.clear();
        if(strip) strip->clear();
        breakStartTime = millis();
        breakDuration = random16(1000, 4000); 
        fireworkStep = BREAK_BETWEEN;
      }
      break;

    case BREAK_BETWEEN:
      if (millis() - breakStartTime >= breakDuration) {
        fireworkStep = LAUNCH_INIT;
      }
      break;
  }

  return 25; 
}
