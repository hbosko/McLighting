void client_state() {

  if (client_loged_out != client_loged_out_last) {
      client_loged_out_last = client_loged_out;
    notifyClients(notify_evt,"clients logged  out");
    DEBUG_PRINT("=> client_loged_out: ");
    DEBUG_PRINTLN(client_loged_out);
  }

  if (manual_reboot_req != manual_reboot_req_last) {
      manual_reboot_req_last = manual_reboot_req;
    notifyClients(notify_evt,"manual_reboot");
    DEBUG_PRINT("=> manual_reboot_req: ");
    DEBUG_PRINTLN(manual_reboot_req);
  }

  if (HTTP_client != HTTP_client_last) {
      HTTP_client_last = HTTP_client;
      String c = "HTTP_client: " + HTTP_client;
    notifyClients(notify_evt,c);
    DEBUG_PRINTLN(c);
  }

  if (LOGIN_client != LOGIN_client_last) {
      LOGIN_client_last = LOGIN_client;
      String c = "LOGIN_client: " + LOGIN_client;
    notifyClients(notify_evt,c);
    DEBUG_PRINTLN(c);
  }

  if (LOGOUT_client != LOGOUT_client_last) {
      LOGOUT_client_last = LOGOUT_client;
      String c = "LOGOUT_client: " + LOGOUT_client;
    notifyClients(notify_evt,c);
    DEBUG_PRINTLN(c);
  }

  if (Settings_client != Settings_client_last) {
      Settings_client_last = Settings_client;
      String c = "Settings_client: " + Settings_client;
    notifyClients(notify_evt,c);
    DEBUG_PRINTLN(c);
  }

  if (NotFound_client != NotFound_client_last) {
      NotFound_client_last = NotFound_client;
      String c = "NotFound_client: " + NotFound_client;
    notifyClients(notify_evt,c);
    DEBUG_PRINTLN(c);
  }

  if (wifi_edit_client != wifi_edit_client_last) {
      wifi_edit_client_last = wifi_edit_client;
      String c = "wifi_edit_client: " + wifi_edit_client;
    notifyClients(notify_evt,c);
    DEBUG_PRINTLN(c);
  }

  if (MQTT_client != MQTT_client_last) {
      MQTT_client_last = MQTT_client;
      String c = "MQTT_client: " + MQTT_client;
    notifyClients(notify_evt,c);
    DEBUG_PRINTLN(c);
  }

  if (WS_client != WS_client_last) {
      WS_client_last = WS_client;
      String c = "WS_client: " + WS_client;
    notifyClients(notify_evt,c);
    DEBUG_PRINTLN(c);
  }
}
