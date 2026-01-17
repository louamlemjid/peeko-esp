#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#define MAX_LEN 32  // max length for each string
#define EEPROM_ADDR_FIRSTNAME 192
#define EEPROM_ADDR_LASTNAME 256
#define EEPROM_ADDR_PEEKONAME 320
#define SERVERLINK "https://peeko-nine.vercel.app"

void fetchPeekoMood(const String &peekoCode, void (*callback)(String mood));
void saveUserData(String firstName, String lastName, String peekoName);
void createPeeko(String peekoCode, String peekoName = "");
void downloadAnimationSet(JsonArray animation);
bool saveToSpiffs(const String &url, const String &fileName);
String fetchAnimationLink(const String& peekoCode);
void updateWeather();
void fetchIncomingMessage(const String &peekoCode);
void fetchIncomingImageToBuffer(const String& imageUrl);