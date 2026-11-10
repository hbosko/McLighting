void TriggerStripSave_loop() {

  if (triggerSaveStripConfig) {
    triggerSaveStripConfig = false;
    save_strip_config_ticker.detach();
      write_strip_config_FS();
        notifyClients(notify_evt, "Saved Strip CONFIG");
  }

  if (triggerSaveStripState) {
    triggerSaveStripState = false;
    save_strip_state_ticker.detach();
      write_strip_state_FS();
        notifyClients(notify_evt, "Saved Strip STATE");
  }
}

void handle_strip_state(AsyncWebServerRequest * request) {
  bool updateStrip = false;

    int params = request->params();
      for (int i = 0; i < params; i++) {
      const AsyncWebParameter* p = request->getParam(i);
      DEBUG_PRINTF("Param name: %s | value: %s\r\n", p->name().c_str(), p->value().c_str());
    }

  if (request->hasParam("ct", true)) {
    uint16_t pixelCt = request->getParam("ct", true)->value().toInt();
    if (pixelCt > 0 && pixelCt != WS2812FXStripSettings.stripSize) {
      WS2812FXStripSettings.stripSize = pixelCt;
      updateStrip = true;
      notifyClients(notify_evt, notify_OK);
    }
  }

  if (request->hasParam("rgbo", true)) {
    String RGBOrder = request->getParam("rgbo", true)->value();
         if (RGBOrder == F("grb")) { WS2812FXStripSettings.RGBOrder = NEO_GRB; updateStrip = true; }
    else if (RGBOrder == F("gbr")) { WS2812FXStripSettings.RGBOrder = NEO_GBR; updateStrip = true; }
    else if (RGBOrder == F("rgb")) { WS2812FXStripSettings.RGBOrder = NEO_RGB; updateStrip = true; }
    else if (RGBOrder == F("rbg")) { WS2812FXStripSettings.RGBOrder = NEO_RBG; updateStrip = true; }
    else if (RGBOrder == F("brg")) { WS2812FXStripSettings.RGBOrder = NEO_BRG; updateStrip = true; }
    else if (RGBOrder == F("bgr")) { WS2812FXStripSettings.RGBOrder = NEO_BGR; updateStrip = true; }
  }

  if (request->hasParam("pin", true)) {
    uint8_t pin = request->getParam("pin", true)->value().toInt();
    
    if (pin == 16 || pin == 5 || pin == 4 || pin == 0 || pin == 2 || pin == 14 || pin == 12 || pin == 13 || pin == 15 || pin == 3 || pin == 1) {
      if (WS2812FXStripSettings.pin != pin) {
        WS2812FXStripSettings.pin = pin;
        updateStrip = true;
      }
    }
  }

  if (request->hasParam("pixtype", true)) {
    uint8_t pix = request->getParam("pixtype", true)->value().toInt();
    if (WS2812FXStripSettings.pixelType != pix) {
      WS2812FXStripSettings.pixelType = pix;
      updateStrip = true;
    }
  }

  if (updateStrip) {
    if (strip) {
      ws2812fx_mode = strip->getMode();
      if (strip->isRunning()) strip->stop();
      
      init_Strip();
      
      strip->setMode(ws2812fx_mode);
      strip->trigger();
    }
    
      save_strip_config_ticker.detach();
      save_strip_config_ticker.once(save_ticker_config_time, Trigger_Save_Config);

      DEBUG_PRINTLN("Strip cfg posodobljen");
  }

  request->send(200, "application/json", "{\"status\":\"success\"}");
}
