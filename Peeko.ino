#include <EEPROM.h>
#include "web.h"
#include "display.h"
#include "fetch.h"
#include "app_state.h"
#include "animation.h"
#include <time.h>
#include "EyesTouchSensor.h"
#include "sound.h"
#include <FS.h>
#include <SPIFFS.h>
#include "updatePeeko.h"

PeekoUpdate peekoUpdater;

Sound peekoSound;

#define EEPROM_SIZE 512   // safe size
// #define EEPROM_ADDR_SSID 0
// #define EEPROM_ADDR_PASS 64
// #define EEPROM_ADDR_CODE 128
#define MAX_LEN 32

String ssid = "";
String password = "";
String peekoCode = "";
String firstName = "";
String lastName = "";
String peekoName = "";

unsigned long lastFetch = 0;

//==== PeekoDoro ====
PeekoDoro peekoDoro;

// ==== Touch =====
#define TOUCH_PIN 10
PeekoMoodSensor peekoMoodSensor(TOUCH_PIN);
MenuTouchSensor menuTouchSensor(TOUCH_PIN);
AnimationTouchSensor animationTouchSensor(TOUCH_PIN);
ClockTouchSensor clockTouchSensor(TOUCH_PIN);
WeatherTouchSensor weatherTouchSensor(TOUCH_PIN);
PeekoDoroTouchSensor peekoDoroTouchSensor(TOUCH_PIN);
MessageTouchSensor messageTouchSensor(TOUCH_PIN);
UpdaterTouchSensor updaterTouchSensor(TOUCH_PIN);

// ===== TIME ======
#define GMT_OFFSET_SEC  3600   // +1 hour
#define DAYLIGHT_OFFSET_SEC 0

void initTime() {
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, "pool.ntp.org");
}


// ===== EEPROM helpers =====
String readStringFromEEPROM(int startAddr, int maxLen) {
    char c;
    String str = "";
    for (int i = 0; i < maxLen; i++) {
        c = EEPROM.read(startAddr + i);
        if (c == 0) break;
        str += c;
    }
    return str;
}

void writeStringToEEPROM(int startAddr, const String &str, int maxLen) {
    for (int i = 0; i < maxLen; i++) {
        if (i < str.length()) EEPROM.write(startAddr + i, str[i]);
        else EEPROM.write(startAddr + i, 0);
    }
    EEPROM.commit();
}

// ===== Display callback =====
void displayMoodCallback(String mood) {
    moodUpdate(mood);           // update RoboEyes
    
}

// ===== Setup =====
void setup() {
    Serial.begin(115200);
    peekoMoodSensor.begin();
    pinMode(SPEAKER_PIN, OUTPUT);
     
    displayInit();
    EEPROM.begin(EEPROM_SIZE);
    
    
    // Read Wi-Fi and Peeko code from EEPROM
    ssid = readStringFromEEPROM(EEPROM_ADDR_SSID, MAX_LEN);
    password = readStringFromEEPROM(EEPROM_ADDR_PASS, MAX_LEN);
    peekoCode = readStringFromEEPROM(EEPROM_ADDR_CODE, MAX_LEN);
    
    
    displayWelcome();
    delay(7000);
    if (!SPIFFS.begin(true)) {
        displayWifi("SPIFFS Mount Fail");
        delay(1000);
        while (1); // Halt because we can't continue without storage
    }
    
    displayWifi("SPIFFS OK");
    delay(1000);
    if (ssid.length() > 0 && password.length() > 0) {
        // Display connecting message
        displayWifiIcon();

        WiFi.begin(ssid.c_str(), password.c_str());

        unsigned long startAttempt = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 15000) {
            delay(500);
            messageUpdate();  // update OLED/RoboEyes with connecting message

        }

        if (WiFi.status() == WL_CONNECTED) {

            displayWifi("Wi-Fi Connected!");
            // fetchAnimationLink(peekoCode);
            peekoUpdater.loadInformations();
            initTime();
            fetchPeekoMood(peekoCode, displayMoodCallback);
            delay(2000); 
        } else {
            
            displayWifi("Wi-Fi Failed!\nStarting AP...");
            delay(2000);
            WiFi.softAP("Peeko");
            webInit();
        }
    } else {
        
        displayWifi("No Wi-Fi Saved\nStarting AP...");
        delay(2000);
        WiFi.softAP("PeekoSetup");
        webInit();
    }
    peekoName = readStringFromEEPROM(EEPROM_ADDR_PEEKONAME, MAX_LEN);
    firstName = readStringFromEEPROM(EEPROM_ADDR_FIRSTNAME, MAX_LEN);
    lastName = readStringFromEEPROM(EEPROM_ADDR_LASTNAME, MAX_LEN);

    if(peekoName && firstName && lastName){
        displayGreeting(firstName,lastName,peekoName);
        delay(5000);
    }
}


// ===== Main loop =====
void loop() {
    server.handleClient();
    if(digitalRead(1) == HIGH){
    
        displayWifi("1");
        delay(3000);
    }
    
    // Fetch Peeko mood every 20 seconds
    if (millis() - lastFetch > 30000 && peekoCode.length() > 0 && WiFi.status() == WL_CONNECTED) {
        fetchIncomingMessage(peekoCode);
        lastFetch = millis();
    }

    loopDisplay(); 
    peekoSound.update();
    // peekoMoodSensor.update();
    peekoDoro.update();
}
