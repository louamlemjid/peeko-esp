#include <WebServer.h>
#include <EEPROM.h>
#define MAX_LEN 32  // max length for each string
#define EEPROM_ADDR_SSID 0
#define EEPROM_ADDR_PASS 64
#define EEPROM_ADDR_CODE 128

extern WebServer server;

void webInit();
void saveWiFiConfig(String ssid, String password, String code);
