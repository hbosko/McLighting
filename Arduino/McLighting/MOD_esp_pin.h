
#ifdef ENABLE_BLINK

  const char* ESP_pin_name[] PROGMEM = {
    "ESP_led",
    "ESP_free"
  };

  const uint8_t ESP_pin[] = { LED_BUILTIN, 2 };
  const uint8_t ESP_pin_size = sizeof(ESP_pin) / sizeof(ESP_pin[0]);

  // NOt good!! avoid!!
  /* /
    uint8_t ESP_buzzer = ESP_pin[0];
    uint8_t ESP_led = ESP_pin[1];
  */

    const uint8_t ESP_led = 0;
    const uint8_t ESP_buzzer = 1;

    unsigned long esp_LED_pm[ESP_pin_size];
    uint8_t esp_LED_state[ESP_pin_size];
    bool esp_LED_blink_enabled[ESP_pin_size];
    uint8_t esp_LED_mode[ESP_pin_size];
    uint8_t esp_LED_counting[ESP_pin_size];
    bool esp_LED_isPause[ESP_pin_size];

#endif