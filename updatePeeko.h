#pragma once

#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include "display.h"  // your display function

#define EEPROM_SIZE 512
#define EEPROM_ADDR_VERSION 384 // float version storage
#define EEPROM_ADDR_VERSIONNAME 448 // float version storage

class PeekoUpdate {
private:
    float _versionNumber;
    String _versionName;

public:
    // Base URL of your API
    const char* versionApiUrl = "https://peeko-nine.vercel.app/api/v1/version/latestVersion";

    PeekoUpdate():_versionNumber(0.0),_versionName("Lab") {}

    // --- Version getters and setters ---
    void setVersionNumber(float version) { _versionNumber = version; }
    float getVersionNumber() { return _versionNumber; }

    void setVersionName(const String& name) { _versionName = name; }
    String getVersionName() { return _versionName; }

    void loadInformations(){
        float versionNumber = readVersion();
        String versionName = readVersionName();

        setVersionNumber(versionNumber);
        setVersionName(versionName);
    }
    // --- EEPROM helpers ---
    void storeVersion(float version) {
        byte* p = (byte*)(void*)&version;
        for (int i = 0; i < sizeof(version); i++) {
            EEPROM.write(EEPROM_ADDR_VERSION + i, p[i]);
        }
        EEPROM.commit();
        setVersionNumber(version); // update internal attribute
    }

    float readVersion() {
        float version = 0;
        byte* p = (byte*)(void*)&version;
        for (int i = 0; i < sizeof(version); i++) {
            p[i] = EEPROM.read(EEPROM_ADDR_VERSION + i);
        }
        setVersionNumber(version); // update internal attribute
        return version;
    }

    void storeVersionName(String versionName) {
        byte* p = (byte*)(void*)&versionName;
        for (int i = 0; i < sizeof(versionName); i++) {
            EEPROM.write(EEPROM_ADDR_VERSIONNAME + i, p[i]);
        }
        EEPROM.commit();
        setVersionName(versionName); // update internal attribute
    }

    String readVersionName() {
        String versionName = "";
        byte* p = (byte*)(void*)&versionName;
        for (int i = 0; i < sizeof(versionName); i++) {
            p[i] = EEPROM.read(EEPROM_ADDR_VERSIONNAME + i);
        }
        setVersionName(versionName); // update internal attribute
        return versionName;
    }

    // --- Fetch latest version JSON from API ---
    bool fetchLatestVersion(float &serverVersion, String &binLink, String &versionName) {
        if (WiFi.status() != WL_CONNECTED) {
            displayWifi("[OTA] WiFi not connected");
            return false;
        }

        HTTPClient http;
        http.begin(versionApiUrl);
        int httpCode = http.GET();
        if (httpCode != HTTP_CODE_OK) {
            displayWifi("[OTA] HTTP error: " + String(httpCode));
            http.end();
            return false;
        }

        String payload = http.getString();
        http.end();

        StaticJsonDocument<512> doc;
        DeserializationError err = deserializeJson(doc, payload);
        if (err) {
            displayWifi("[OTA] JSON parse error");
            return false;
        }

        // parse the JSON
        if (!doc["success"].as<bool>()) {
            displayWifi("[OTA] API returned error");
            return false;
        }

        serverVersion = doc["version"]["number"].as<float>();
        binLink = doc["version"]["link"].as<String>();
        versionName = doc["version"]["name"].as<String>();

        // setVersionNumber(serverVersion); // update internal attribute
        // setVersionName(versionName);     // update internal attribute

        return true;
    }

    // --- Perform OTA update if needed ---
    void update() {
        float serverVersion;
        String binLink;
        String serverName;

        if (!fetchLatestVersion(serverVersion, binLink, serverName)) return;

        // float currentVersion = readVersion();
        displayWifi("[OTA] Current version: " + String(_versionNumber) + " (" + _versionName + ")");
        displayWifi("[OTA] Latest version: " + String(serverVersion) + " (" + serverName + ")");

        if (_versionNumber == serverVersion) {
            displayWifi("[OTA] Firmware is up-to-date.");
            delay(1500);
            return;
        }

        displayWifi("[OTA] New version detected, updating...");
        delay(1500);

        HTTPClient http;
        http.begin(binLink);
        int httpCode = http.GET();
        if (httpCode != HTTP_CODE_OK) {
            displayWifi("[OTA] HTTP error: " + String(httpCode));
            http.end();
            return;
        }

        int contentLength = http.getSize();
        if (contentLength <= 0) {
            displayWifi("[OTA] Invalid content length");
            http.end();
            return;
        }

        displayWifi("[OTA] Firmware size: " + String(contentLength) + " bytes");
        delay(1000);
        if (!Update.begin(contentLength)) {
            displayWifi("[OTA] Not enough space for OTA");
            http.end();
            return;
        }

        displayWifi("[OTA] Starting firmware update...");
        WiFiClient* stream = http.getStreamPtr();
        size_t written = Update.writeStream(*stream);

        if (written != contentLength) {
            displayWifi("[OTA] Written " + String(written) + " / " + String(contentLength) + " bytes");
        }

        if (!Update.end()) {
            displayWifi("[OTA] Update failed: " + String(Update.errorString()));
            http.end();
            return;
        }

        if (Update.isFinished()) {
            displayWifi("[OTA] Update successful! Peeko is Rebooting...");
            storeVersion(serverVersion); // store new version
            storeVersionName(serverName);  // update name
            delay(1000);
            ESP.restart();
        } else {
            displayWifi("[OTA] Update not finished");
        }

        http.end();
    }
};
