// u messed up again.. palettes not loading corectly ! ;( fix somedays...

void strip_service_loop(){
  switch (Mode) {
    case SET_MODE:
      if (strip) {
        strip->setMode(ws2812fx_mode);
        strip->trigger();
      }
      prevmode = SET_MODE;
      Mode = SETCOLOR;
      break;

    case OFF:
      if (strip && strip->isRunning()) {
        strip->stop();
      }
      break;

        case SETCOLOR:
      if (strip) {
        if (isCustomPaletteActive == true) {
          if (paletteIdx > 0) { 
            WS2812FX::Segment* seg = strip->getSegment(0);
            
            if (seg != nullptr) {
              if (WS2812FXStripState.isCustomPal) {
                seg->colors[0] = getCustomPaletteColor(paletteIdx, 0);
                seg->colors[1] = getCustomPaletteColor(paletteIdx, 1);
                seg->colors[2] = getCustomPaletteColor(paletteIdx, 2);
              } 
              else {
                if (paletteIdx < BUILTIN_PALETTE_COUNT) {
                  seg->colors[0] = pgm_read_dword(&(BuiltInPalettes[paletteIdx][0]));
                  seg->colors[1] = pgm_read_dword(&(BuiltInPalettes[paletteIdx][1]));
                  seg->colors[2] = pgm_read_dword(&(BuiltInPalettes[paletteIdx][2]));
                }
              }
            }
          } 
        } 
        else if (isCustomPaletteActive == false) {
          strip->setColor(main_color.red, main_color.green, main_color.blue);
        }
        
        strip->trigger();
        strip->show();
      }
      Mode = (prevmode == SET_MODE) ? SETSPEED : HOLD;
      break;

    case SETSPEED:
      if (strip) {
        strip->setSpeed(convertSpeed(ws2812fx_speed));
        strip->trigger();
      }
      Mode = (prevmode == SET_MODE) ? BRIGHTNESS : HOLD;
      break;

    case BRIGHTNESS:
      if (strip) {
        strip->setBrightness(brightness);
        strip->trigger();
      }
      if (prevmode == SET_MODE) prevmode = HOLD;
      Mode = HOLD;
      break;

    default:
      break;
  }

  if (strip) strip->service();
}
