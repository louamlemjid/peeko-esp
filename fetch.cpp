#include "fetch.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "app_state.h"
#include "display.h"
#include <EEPROM.h>

// #define EEPROM_ADDR_FIRSTNAME 192
// #define EEPROM_ADDR_LASTNAME 256
// #define EEPROM_ADDR_PEEKONAME 320

void saveUserData(String firstName, String lastName, String peekoName){
    

    // Write firstName
    for (int i = 0; i < MAX_LEN; i++) {
        if (i < firstName.length()) EEPROM.write(EEPROM_ADDR_FIRSTNAME + i, firstName[i]);
        else EEPROM.write(EEPROM_ADDR_FIRSTNAME + i, 0);
    }

    // Write lastName
    for (int i = 0; i < MAX_LEN; i++) {
        if (i < lastName.length()) EEPROM.write(EEPROM_ADDR_LASTNAME + i, lastName[i]);
        else EEPROM.write(EEPROM_ADDR_LASTNAME + i, 0);
    }

    // Write Peeko name
    for (int i = 0; i < MAX_LEN; i++) {
        if (i < peekoName.length()) EEPROM.write(EEPROM_ADDR_PEEKONAME + i, peekoName[i]);
        else EEPROM.write(EEPROM_ADDR_PEEKONAME + i, 0);
    }

    EEPROM.commit();


}

void fetchPeekoMood(const String &peekoCode, void (*callback)(String mood)) {
    if (WiFi.status() != WL_CONNECTED) {
        displayWifi("Wi-Fi not connected");
        return;
    }

    HTTPClient http;
    String url = "https://peeko-nine.vercel.app/api/v1/peeko/" + peekoCode;
    http.begin(url);

    // messageStart("Connecting to server...");

    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, payload);
        if (!error) {
            String mood = doc["peeko"]["mood"] | "DEFAULT";
            String peekoName = doc["peeko"]["peekoName"];

            String firstName = doc["peeko"]["user"]["firstName"];
            String lastName = doc["peeko"]["user"]["lastName"];
            
            saveUserData(firstName,lastName,peekoName);
            displayWifi("Data from server: " + mood + " " + peekoName + " " + firstName + " " + lastName); // briefly show message
            delay(2000); // show for 1s
            if (callback) callback(mood);         // call the provided callback
        } else {
            messageStart("JSON parse error");
        }
    } else {
        messageStart("HTTP error: " + String(httpCode));
    }

    http.end();
}
