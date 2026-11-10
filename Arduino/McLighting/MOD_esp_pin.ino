#ifdef ENABLE_BLINK

void init_ESP_PIN(){
  for (uint8_t i = 0; i < ESP_pin_size; i++) {
    pinMode(ESP_pin[i], OUTPUT);
    digitalWrite(ESP_pin[i], LOW);
  }
}

void ESP_pin_static(const uint8_t pin, uint8_t st) {
  if (pin >= ESP_pin_size) return;
  digitalWrite(ESP_pin[pin], !st);
}


void wifiConnectingLedStatus () {
  if (apActive == true) {
    esp_blink_mode(ESP_led, blink_AP_ACTIVE);
    return;
  }
  if (esp_LED_mode[ESP_led] == blink_MSG) {
    return; 
  }

       if (WiFi.status() == WL_NO_SHIELD) { }
  else if (WiFi.status() == WL_IDLE_STATUS) { }
  else if (WiFi.status() == WL_NO_SSID_AVAIL) { }
  else if (WiFi.status() == WL_SCAN_COMPLETED) { }
  else if (WiFi.status() == WL_CONNECT_FAILED) { }
  else if (WiFi.status() == WL_CONNECTION_LOST) { }
  else if (WiFi.status() == WL_CONNECTED) {
    esp_blink_mode(ESP_led, blink_STANDBY);
  }
  else if (WiFi.status() == WL_DISCONNECTED) {
    esp_blink_mode(ESP_led, blink_DISCONECTED);
  }
}


void esp_blink_mode(const uint8_t pin, uint8_t mode) {
  if (pin >= ESP_pin_size) return;
  
  if (esp_LED_mode[pin] == mode) return;

  esp_LED_mode[pin] = mode;
  esp_LED_counting[pin] = 0;
  esp_LED_isPause[pin] = (mode == 99);

  if (mode == blink_OFF) {
    ESP_pin_static(pin, LOW);
  }
  else if (mode == blink_ON) {
    ESP_pin_static(pin, HIGH);
  }
}

void run_ESP_Blink() {
  for (uint8_t pin = 0; pin < ESP_pin_size; pin++) {
    if (esp_LED_isPause[pin]) continue;

    uint8_t mode = esp_LED_mode[pin];

    if (mode == blink_STANDBY) {
      ESP_Blink_Engine(pin, 50, TEN_SECOND, false);
    }
    else if (mode == blink_MSG) {
      ESP_Blink_Engine(pin, 50, 50, false);
      
      if (esp_LED_counting[pin] >= 3) {
        esp_LED_counting[pin] = 0;
        esp_LED_mode[pin] = blink_OFF;
      }
    }
    else if (mode == blink_AP_ACTIVE) {
      if (esp_LED_counting[pin] <= 2){
        ESP_Blink_Engine(pin, QTR_SECOND, QTR_SECOND, false);
      } else if (esp_LED_counting[pin] > 2) {
        ESP_Blink_Engine(pin, ONE_SECOND, ONE_SECOND, true);
      } 
      if (esp_LED_counting[pin] >= 5){
        esp_LED_counting[pin] = 0; 
      }
    }
    else if (mode == blink_OPEN) {
      ESP_Blink_Engine(pin, TWO_SECOND, QTR_SECOND, false);
    }
    else if (mode == blink_CLOSE) {
      ESP_Blink_Engine(pin, QTR_SECOND, TWO_SECOND, false);
    }
    else if (mode == blink_RUNNING) {
      ESP_Blink_Engine(pin, HALF_SECOND, HALF_SECOND, false);
    }
    else if (mode == blink_WARNING) {
      if (esp_LED_counting[pin] <= 5){
        ESP_Blink_Engine(pin, ONE_SECOND, QTR_SECOND, false);
      } else if (esp_LED_counting[pin] > 5) {
        ESP_Blink_Engine(pin, ONE_SECOND, ONE_SECOND, true); 
      } 
      if (esp_LED_counting[pin] >= 8){
        esp_LED_counting[pin] = 0; 
      }
    }
    else if (mode == blink_CRITICAL) {
      if (esp_LED_counting[pin] <= 5){
        ESP_Blink_Engine(pin, HALF_SECOND, HALF_SECOND, false);
      } else if (esp_LED_counting[pin] > 5) {
        ESP_Blink_Engine(pin, ONE_SECOND, ONE_SECOND, true);
      } 
      if (esp_LED_counting[pin] >= 10){
        esp_LED_counting[pin] = 0;
      }
    }
    else if (mode == blink_ERROR) {
      if (esp_LED_counting[pin] <= 5){
        ESP_Blink_Engine(pin, ONE_SECOND, ONE_SECOND, false);
      } else if (esp_LED_counting[pin] > 5) {
        ESP_Blink_Engine(pin, 100, HALF_SECOND, false);
      } 
      if (esp_LED_counting[pin] >= 20){
        esp_LED_counting[pin] = 0; 
      }
    }
    else if (mode == 9) { 
      if (esp_LED_counting[pin] <= 5){
        ESP_Blink_Engine(pin, 1000, 1000, false);
      } else if (esp_LED_counting[pin] > 5) {
        ESP_Blink_Engine(pin, 1000, 1000, true);
      }
    }
    else if (mode == 20) {
      if (esp_LED_counting[pin] <= 5){
        ESP_Blink_Engine(pin, 1000, 100, false);
      }
      else if ((esp_LED_counting[pin] >= 5) && (esp_LED_counting[pin] < 25)) {
        ESP_Blink_Engine(pin, 100, 100, false);
      }
      else if ((esp_LED_counting[pin] >= 25) && (esp_LED_counting[pin] < 30)) {
        ESP_Blink_Engine(pin, 2000, 2000, false);
      }
      else if ((esp_LED_counting[pin] >= 30) && (esp_LED_counting[pin] < 50)) {
        ESP_Blink_Engine(pin, 50, 100, false);
      }
      else if ((esp_LED_counting[pin] >= 50) && (esp_LED_counting[pin] < 55)) {
        ESP_Blink_Engine(pin, 1000, 1000, true);
      }
      else if ((esp_LED_counting[pin] >= 55) && (esp_LED_counting[pin] < 60)) {
        ESP_Blink_Engine(pin, 2000, 70, false);
      }
      
      if (esp_LED_counting[pin] >= 56){
        esp_LED_counting[pin] = 0; 
      }
    }
  }
}


void ESP_Blink_Engine(const uint8_t fpin, unsigned int OnTime, unsigned int OffTime, bool isPause) {
  unsigned long esp_LED_cm = millis();
  esp_LED_blink_enabled[fpin] = false;

  if((esp_LED_state[fpin] == HIGH) && (esp_LED_cm - esp_LED_pm[fpin] >= OnTime)) {
    esp_LED_state[fpin]  = LOW;  
    esp_LED_pm[fpin] = esp_LED_cm;
    esp_LED_counting[fpin]++;
    esp_LED_blink_enabled[fpin] = true;
  }
  else if ((esp_LED_state[fpin] == LOW) && (esp_LED_cm - esp_LED_pm[fpin] >= OffTime)) {
    esp_LED_state[fpin]  = HIGH;  
    esp_LED_pm[fpin] = esp_LED_cm;
    esp_LED_blink_enabled[fpin] = true;
  }

  if(esp_LED_blink_enabled[fpin] == true) {
    if (isPause == false){
      ESP_pin_static(fpin, esp_LED_state[fpin]);
    }
  }
}

#endif