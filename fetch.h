#pragma once
#include <Arduino.h>
#include <WiFi.h>
#define MAX_LEN 32  // max length for each string
#define EEPROM_ADDR_FIRSTNAME 192
#define EEPROM_ADDR_LASTNAME 256
#define EEPROM_ADDR_PEEKONAME 320

void fetchPeekoMood(const String &peekoCode, void (*callback)(String mood));
void saveUserData(String firstName, String lastName, String peekoName);
