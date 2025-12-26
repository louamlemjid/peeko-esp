#include "web.h"

// #define EEPROM_SIZE 256
// #define EEPROM_ADDR_SSID 0
// #define EEPROM_ADDR_PASS 64
// #define EEPROM_ADDR_CODE 128
#include <EEPROM.h>


const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Peeko Setup</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <style>
    body { font-family: Arial; text-align: center; padding: 20px; background: #8CE4FF; }
    input, button { padding: 10px; margin: 8px; font-size: 16px; border-radius: 8px; border: none; }
    button { background-color: black; color: white; cursor: pointer; }
    button:hover { background-color: #333; }
  </style>
</head>
<body>
  <h2>Peeko Setup</h2>
  <form action="/setup">
    <input name="ssid" placeholder="Wi-Fi SSID" required />
    <input name="password" placeholder="Wi-Fi Password" required />
    <input name="code" placeholder="Peeko Code" required />
    <button type="submit">Save</button>
  </form>
</body>
</html>
)rawliteral";



#include "app_state.h"
#include "display.h"


WebServer server(80);


void saveWiFiConfig(String ssid, String password, String code) {

    // Write SSID
    for (int i = 0; i < MAX_LEN; i++) {
        if (i < ssid.length()) EEPROM.write(EEPROM_ADDR_SSID + i, ssid[i]);
        else EEPROM.write(EEPROM_ADDR_SSID + i, 0);
    }

    // Write password
    for (int i = 0; i < MAX_LEN; i++) {
        if (i < password.length()) EEPROM.write(EEPROM_ADDR_PASS + i, password[i]);
        else EEPROM.write(EEPROM_ADDR_PASS + i, 0);
    }

    // Write Peeko code
    for (int i = 0; i < MAX_LEN; i++) {
        if (i < code.length()) EEPROM.write(EEPROM_ADDR_CODE + i, code[i]);
        else EEPROM.write(EEPROM_ADDR_CODE + i, 0);
    }

    EEPROM.commit();
    Serial.println("Wi-Fi and Peeko config saved!");
}


void webInit() {
  server.on("/", []() { server.send(200, "text/html", HTML_PAGE); });

  server.on("/setup", []() {
    if (server.hasArg("ssid") && server.hasArg("password") && server.hasArg("code")) {
      String ssid = server.arg("ssid");
      String password = server.arg("password");
      String code = server.arg("code");
      messageStart(ssid+password+code);
      saveWiFiConfig(ssid, password, code);

      server.send(200, "text/html", "<h2>Saved! Reboot to connect...</h2>");
    } else {
      server.send(400, "text/html", "<h2>Missing fields</h2>");
    }
  });

  server.begin();
}
