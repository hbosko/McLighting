void client_state() {
  char buf[64]; 

  if (client_loged_out != client_loged_out_last) {
    client_loged_out_last = client_loged_out;
    notifyClients(notify_evt, "clients logged  out");
    DEBUG_PRINT("=> client_loged_out: ");
    DEBUG_PRINTLN(client_loged_out);
  }

  if (manual_reboot_req != manual_reboot_req_last) {
    manual_reboot_req_last = manual_reboot_req;
    notifyClients(notify_evt, "manual_reboot");
    DEBUG_PRINT("=> manual_reboot_req: ");
    DEBUG_PRINTLN(manual_reboot_req);
  }

  if (strcmp(HTTP_client, HTTP_client_last) != 0) {
    strcpy(HTTP_client_last, HTTP_client);
    snprintf(buf, sizeof(buf), "HTTP_client: %s", HTTP_client);
    notifyClients(notify_evt, buf);
    DEBUG_PRINTLN(buf);
  }

  if (strcmp(LOGIN_client, LOGIN_client_last) != 0) {
    strcpy(LOGIN_client_last, LOGIN_client);
    snprintf(buf, sizeof(buf), "LOGIN_client: %s", LOGIN_client);
    notifyClients(notify_evt, buf);
    DEBUG_PRINTLN(buf);
  }

  if (strcmp(LOGOUT_client, LOGOUT_client_last) != 0) {
    strcpy(LOGOUT_client_last, LOGOUT_client);
    snprintf(buf, sizeof(buf), "LOGOUT_client: %s", LOGOUT_client);
    notifyClients(notify_evt, buf);
    DEBUG_PRINTLN(buf);
  }

  if (strcmp(Settings_client, Settings_client_last) != 0) {
    strcpy(Settings_client_last, Settings_client);
    snprintf(buf, sizeof(buf), "Settings_client: %s", Settings_client);
    notifyClients(notify_evt, buf);
    DEBUG_PRINTLN(buf);
  }

  if (strcmp(NotFound_client, NotFound_client_last) != 0) {
    strcpy(NotFound_client_last, NotFound_client);
    snprintf(buf, sizeof(buf), "NotFound_client: %s", NotFound_client);
    notifyClients(notify_evt, buf);
    DEBUG_PRINTLN(buf);
  }

  if (strcmp(wifi_edit_client, wifi_edit_client_last) != 0) {
    strcpy(wifi_edit_client_last, wifi_edit_client);
    snprintf(buf, sizeof(buf), "wifi_edit_client: %s", wifi_edit_client);
    notifyClients(notify_evt, buf);
    DEBUG_PRINTLN(buf);
  }

  if (strcmp(MQTT_client, MQTT_client_last) != 0) {
    strcpy(MQTT_client_last, MQTT_client);
    snprintf(buf, sizeof(buf), "MQTT_client: %s", MQTT_client);
    notifyClients(notify_evt, buf);
    DEBUG_PRINTLN(buf);
  }

  if (strcmp(WS_client, WS_client_last) != 0) {
    strcpy(WS_client_last, WS_client);
    snprintf(buf, sizeof(buf), "WS_client: %s", WS_client);
    notifyClients(notify_evt, buf);
    DEBUG_PRINTLN(buf);
  }
}
