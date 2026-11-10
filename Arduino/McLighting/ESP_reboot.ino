unsigned long RBT_previousMillis = 0;
unsigned long RBT_startTime = 0;
unsigned long RBT_elapsedTime = 0;
bool RBT_enabled = false;

void esp_reboot_timer() {
  unsigned long RBT_currentMillis = millis();

  if (esp_reboot == true) {
    RBT_previousMillis = RBT_currentMillis;
    RBT_startTime = RBT_currentMillis;

    DEBUG_PRINT("esp_reboot -> 5 sekund. Trenutni čas: "); 
    DEBUG_PRINTLN(RBT_currentMillis / 1000); 
      notifyClients(notify_evt, "Reboot in 5 sec..");

    RBT_enabled = true;
    esp_reboot = false;
  }

  if (RBT_enabled) {
    if (RBT_currentMillis - RBT_previousMillis >= FIVE_SECOND) {
      RBT_enabled = false;
      RBT_elapsedTime = RBT_currentMillis - RBT_startTime;

      DEBUG_PRINT("Was ON for Millis: ");
      DEBUG_PRINT(RBT_elapsedTime);
      DEBUG_PRINT(" or Seconds: ");
      DEBUG_PRINTLN(RBT_elapsedTime / Seconds);
        notifyClients(notify_evt, "ESP rebooting NOW..");
      
      delay(100); 
        ESP_REBOOT;
    }
  }
}