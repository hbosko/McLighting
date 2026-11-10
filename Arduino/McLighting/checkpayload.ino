#if defined(ESP8266)
  #include "core_esp8266_features.h"
#elif defined(ESP32)

#endif

void autoTick() {
  strip->setColor(autoParams[autoCount][0]);
  strip->setSpeed(convertSpeed((char)autoParams[autoCount][1]));
  strip->setMode((char)autoParams[autoCount][2]);
  autoTicker.once_ms((uint32_t)autoParams[autoCount][3], autoTick);
//  DBG_OUTPUT_PORT.printf("autoTick[%d]: {0x%06x, %d, %d, %d}\n", autoCount, autoParams[autoCount][0], (char)autoParams[autoCount][1], (char)autoParams[autoCount][2], (uint32_t)autoParams[autoCount][3]);

  autoCount++;
  if (autoCount >= (sizeof(autoParams) / sizeof(autoParams[0]))) autoCount = 0;
}

void handleAutoStart() {
  autoCount = 0;
  autoTick();
  strip->start();
}

void handleAutoStop() {
  autoTicker.detach();
  strip->stop();
}

void handleSetWS2812FXMode(const char * mypayload) {
  Mode = SET_MODE;
  char ws2812fx_mode_tmp = (char) strtol((const char *) &mypayload[1], NULL, 10);
  ws2812fx_mode = constrain(ws2812fx_mode_tmp, 0, strip->getModeCount() - 1);
}

#ifdef ENABLE_LEGACY_ANIMATIONS
  void handleSetNamedMode(String str_mode) {
    exit_func = true;
  
    if (str_mode.startsWith("=off")) {
      Mode = OFF;
    }
    if (str_mode.startsWith("=all")) {
      ws2812fx_mode = FX_MODE_STATIC;
      Mode = SET_MODE;
    }
    if (str_mode.startsWith("=wipe")) {
      Mode = WIPE;
    }
    if (str_mode.startsWith("=rainbow")) {
      Mode = RAINBOW;
    }
    if (str_mode.startsWith("=rainbowCycle")) {
      Mode = RAINBOWCYCLE;
    }
    if (str_mode.startsWith("=theaterchase")) {
      Mode = THEATERCHASE;
    }
    if (str_mode.startsWith("=twinkleRandom")) {
      Mode = TWINKLERANDOM;
    }
    if (str_mode.startsWith("=theaterchaseRainbow")) {
      Mode = THEATERCHASERAINBOW;
    }
    if (str_mode.startsWith("=tv")) {
      Mode = TV;
    }
  }
#endif

// ***************************************************************************
// Handler functions for WS
// ***************************************************************************
void handleSetMainColor(const char* mypayload) {
  // decode rgb data
  uint32_t rgb = (uint32_t) strtol((const char *) &mypayload[1], NULL, 16);
  main_color.red = ((rgb >> 16) & 0xFF);
  main_color.green = ((rgb >> 8) & 0xFF);
  main_color.blue = ((rgb >> 0) & 0xFF);

  if (strip) {
    strip->setColor(main_color.red, main_color.green, main_color.blue);
  }

  Mode = SETCOLOR;
}

void handleSetAllMode(const char * mypayload) {
  // decode rgb data
  uint32_t rgb = (uint32_t) strtol((const char *) &mypayload[1], NULL, 16);

  main_color.red = ((rgb >> 16) & 0xFF);
  main_color.green = ((rgb >> 8) & 0xFF);
  main_color.blue = ((rgb >> 0) & 0xFF);

//  DEBUG_PRINTLN("WS: Set all leds to main color: [%u] [%u] [%u]\n", main_color.red, main_color.green, main_color.blue);
  #ifdef ENABLE_LEGACY_ANIMATIONS
    exit_func = true;
  #endif
  ws2812fx_mode = FX_MODE_STATIC;
  Mode = SET_MODE;
}

void handleSetSingleLED(const char * mypayload, char firstChar = 0) {
  // decode led index
  char templed[3];
  strncpy (templed, (const char *) &mypayload[firstChar], 2 );
  char led = atoi(templed);

//  DEBUG_PRINTLN("led value: [%i]. Entry threshold: <= [%i] (=> %s)\n", led, strip->numPixels(), mypayload );
  if (led <= strip->numPixels()) {
    char redhex[3];
    char greenhex[3];
    char bluehex[3];
    strncpy (redhex, (const char *) &mypayload[2 + firstChar], 2 );
    strncpy (greenhex, (const char *) &mypayload[4 + firstChar], 2 );
    strncpy (bluehex, (const char *) &mypayload[6 + firstChar], 2 );
    redhex[2] = 0x00;
    greenhex[2] = 0x00;
    bluehex[2] = 0x00;
    ledstates[led].red =   strtol(redhex, NULL, 16);
    ledstates[led].green = strtol(greenhex, NULL, 16);
    ledstates[led].blue =  strtol(bluehex, NULL, 16);
//    DEBUG_PRINTLN("rgb.red: [%s] rgb.green: [%s] rgb.blue: [%s]\n", redhex, greenhex, bluehex);
//    DEBUG_PRINTLN("rgb.red: [%i] rgb.green: [%i] rgb.blue: [%i]\n", strtol(redhex, NULL, 16), strtol(greenhex, NULL, 16), strtol(bluehex, NULL, 16));
//    DEBUG_PRINTLN("WS: Set single led [%i] to [%i] [%i] [%i] (%s)!\n", led, ledstates[led].red, ledstates[led].green, ledstates[led].blue, mypayload);


    strip->setPixelColor(led, ledstates[led].red, ledstates[led].green, ledstates[led].blue);
    strip->show();
  }
  #ifdef ENABLE_LEGACY_ANIMATIONS
    exit_func = true;
  #endif
  Mode = CUSTOM;
}

void handleSetDifferentColors(const char * mypayload) {
  char* nextCommand = 0;
  nextCommand = (char*) strtok((char*) mypayload, "+");
  while (nextCommand) {
    handleSetSingleLED(nextCommand, 0);
    nextCommand = (char*) strtok(NULL, "+");
  }
}

void handleRangeDifferentColors(const char * mypayload) {
  char* nextCommand = 0;
  nextCommand = (char*) strtok((char*) mypayload, "R");

  while (nextCommand) {
    // Loop for each LED.
    char startled[3] = { 0, 0, 0 };
    char endled[3] = { 0, 0, 0 };
    char colorval[7] = { 0, 0, 0, 0, 0, 0, 0 };
    strncpy ( startled, (const char *) &nextCommand[0], 2 );
    strncpy ( endled, (const char *) &nextCommand[2], 2 );
    strncpy ( colorval, (const char *) &nextCommand[4], 6 );
    int rangebegin = atoi(startled);
    int rangeend = atoi(endled);
//    DEBUG_PRINTLN("Setting RANGE from [%i] to [%i] as color [%s] \n", rangebegin, rangeend, colorval);

    while ( rangebegin <= rangeend ) {
      char rangeData[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
      if ( rangebegin < 10 ) {
        // Create the valid 'nextCommand' structure
        sprintf(rangeData, "0%d%s", rangebegin, colorval);
      }
      if ( rangebegin >= 10 ) {
        // Create the valid 'nextCommand' structure
        sprintf(rangeData, "%d%s", rangebegin, colorval);
      }
      // Set one LED
      handleSetSingleLED((char*) rangeData, 0);
      rangebegin++;
    }

    // Next Range at R
    nextCommand = (char*) strtok(NULL, "R");
  }
}


void CheckPayload(const char* payload, size_t len) {

  if (len == 0 || payload == nullptr) return;

  //UDP
  if (WS2812FXStripSettings.udpMaster && payload[0] != '$' && payload[0] != '~' && strstr(payload, "reboot") == nullptr) {
    
    if (millis() - lastUdpSendTime > 10) { 
      //sendUdpBroadcast(payload, len);
      sendUdpUnicastToGroup(payload, len); // send data to with same "boardType"
    }
  }

  if (payload[0] == '#') {
      isCustomPaletteActive = false;
    handleSetMainColor(payload);

      DEBUG_PRINT("Set main color to:: ");
      DEBUG_PRINT(main_color.red);
      DEBUG_PRINT(main_color.green);
      DEBUG_PRINT(main_color.blue);
      DEBUG_PRINTLN();

        save_strip_state_ticker.detach();
        save_strip_state_ticker.once(save_ticker_state_time, Trigger_Save_State);
  }

  // ? ==> Nastavi hitrost
  else if (payload[0] == '?') {
    long d = strtol(&payload[1], NULL, 10);
    
    ws2812fx_speed = constrain(d, 0, 255);
    Mode = SETSPEED;

    DEBUG_PRINT("SPEED set to: ");
    DEBUG_PRINTLN(ws2812fx_speed);

    //notifyClients(notify_evt, notify_OK); // DONT sPAM!

    save_strip_state_ticker.detach();
    save_strip_state_ticker.once(save_ticker_state_time, []() {
      if (strip) {
        strip->setSpeed(convertSpeed(ws2812fx_speed));
      }
      Trigger_Save_State();
    });
  }


  // % ==> Nastavi svetlost
  else if (payload[0] == '%') {
    long b = strtol(&payload[1], NULL, 10);

    brightness = constrain(b, 0, 255);
    Mode = BRIGHTNESS;

    DEBUG_PRINT("Target BRIGHTNESS set to:: ");
    DEBUG_PRINTLN(brightness);
    
    //notifyClients(notify_evt, notify_OK); // DONT sPAM!
    
    save_strip_state_ticker.detach();
    save_strip_state_ticker.once(save_ticker_state_time, []() {
      if (strip) {
        strip->setBrightness(brightness); 
      }
      Trigger_Save_State();
    });
  }


  else if (payload[0] == '*') {
    handleSetAllMode(payload);
      DEBUG_PRINTLN("Set main STATIC color to: ");
    //notifyClients(notify_evt, notify_OK); // DONT sPAM!

      save_strip_state_ticker.detach();
      save_strip_state_ticker.once(save_ticker_state_time, Trigger_Save_State);
  }

  else if (payload[0] == '!') {
    handleSetSingleLED(payload, 1);
      DEBUG_PRINTLN("Set single LED to:: ");
    notifyClients(notify_evt, notify_OK);
    
      save_strip_state_ticker.detach();
      save_strip_state_ticker.once(save_ticker_state_time, Trigger_Save_State);
  }

  else if (payload[0] == '+') {
    handleSetDifferentColors(payload);
      DEBUG_PRINTLN("Set multiple LED to:: ");
    notifyClients(notify_evt, notify_OK);
  }

  else if (payload[0] == 'R') {
    handleRangeDifferentColors(payload);
      DEBUG_PRINTLN("Set range of LEDs to:: ");
    notifyClients(notify_evt, notify_OK);
  }

  #ifdef ENABLE_LEGACY_ANIMATIONS
    else if (payload[0] == '=') {
      handleSetNamedMode(&payload[1]);

        DEBUG_PRINT("Set named Mode to:: ");
        DEBUG_PRINTLN(&payload[1]);

      notifyClients(notify_evt, notify_OK);

        save_strip_state_ticker.detach();
        save_strip_state_ticker.once(save_ticker_state_time, Trigger_Save_State);
    }
  #endif

  else if (payload[0] == '$') {
      DEBUG_PRINTLN("Get status Info");
  }

  else if (payload[0] == '~') {
      DEBUG_PRINTLN("Get WS2812 modes");
  }

  else if (payload[0] == '/') {
    if (strip) strip->start();
    handleSetWS2812FXMode(payload);
    
      DEBUG_PRINT("Set WS2812 Mode: ");
      DEBUG_PRINTLN(payload);
    notifyClients(notify_evt, notify_OK);

      save_strip_state_ticker.detach();
      save_strip_state_ticker.once(save_ticker_state_time, Trigger_Save_State);
  }

  else if (payload[0] == 'p') {
    paletteIdx = atoi(payload + 1);
    
      DEBUG_PRINT("P - paletteIdx: ");
      DEBUG_PRINTLN(paletteIdx);

      isCustomPaletteActive = true;

    if (paletteIdx >= 0 && paletteIdx < BUILTIN_PALETTE_COUNT) {
      WS2812FX::Segment* seg = strip->getSegment(0);
      if (seg != nullptr) {
        seg->colors[0] = pgm_read_dword(&(BuiltInPalettes[paletteIdx][0]));
        seg->colors[1] = pgm_read_dword(&(BuiltInPalettes[paletteIdx][1]));
        seg->colors[2] = pgm_read_dword(&(BuiltInPalettes[paletteIdx][2]));
      }
      if (strip->getMode() == FX_MODE_STATIC) strip->setMode(FX_MODE_PALETTE_STATIC);
      strip->trigger();
      
        save_strip_state_ticker.detach();
        save_strip_state_ticker.once(save_ticker_state_time, Trigger_Save_State);
    }
  }

  else if (payload[0] == 'c') {
    paletteIdx = atoi(payload + 1);

      DEBUG_PRINT("C - paletteIdx: ");
      DEBUG_PRINTLN(paletteIdx);

      isCustomPaletteActive = true;

    if (paletteIdx >= 0 && paletteIdx < actual_custom_palette_count) {
      if (strip->getMode() == FX_MODE_STATIC) {
        strip->setMode(FX_MODE_PALETTE_STATIC); 
      } 

      if (
        strip->getMode() == FX_MODE_PALETTE_COMET ||
        strip->getMode() == FX_MODE_PALETTE_COMET_COLOR ||
        strip->getMode() == FX_MODE_PALETTE_COMET_FOX ||
        strip->getMode() == FX_MODE_PALETTE_FIREWORKS_1D ||
        strip->getMode() == FX_MODE_PALETTE_MULTI_BOUNCE ||
        strip->getMode() == FX_MODE_PALETTE_BOUNCE_SPARKLE ||
        strip->getMode() == FX_MODE_PALETTE_BOUNCE_FADE
      ){
        // leave empty, dont overwrite background
      }
      else {
        WS2812FX::Segment* seg = strip->getSegment(0);
        if (seg != nullptr) {
          seg->colors[0] = getCustomPaletteColor(paletteIdx, 0);
          seg->colors[1] = getCustomPaletteColor(paletteIdx, 1);
          seg->colors[2] = getCustomPaletteColor(paletteIdx, 2);
        }
      }
      strip->trigger();
      
        save_strip_state_ticker.detach();
        save_strip_state_ticker.once(save_ticker_state_time, Trigger_Save_State);
    }
  }

  if (strstr(payload, "reboot") != nullptr) {
      DEBUG_PRINTLN("REbooT WS!");
    esp_reboot = true;
  }


}

