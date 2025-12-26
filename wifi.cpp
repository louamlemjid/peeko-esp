#include "wifi.h"
#include "config.h"

void wifiInit() {
  WiFi.softAP(AP_NAME);
}

