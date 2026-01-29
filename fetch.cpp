#include "fetch.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "app_state.h"
#include "display.h"
#include <EEPROM.h>
#include "server.h"
#include <FS.h>
#include <SPIFFS.h>
#include <WiFiClientSecure.h>
#include "secrets.h"


extern uint8_t incomingImageBuffer[IMAGE_FRAME_SIZE];
extern bool incomingImageReady;


extern float currentTemp;
extern int currentHumidity;

// #define EEPROM_ADDR_FIRSTNAME 192
// #define EEPROM_ADDR_LASTNAME 256
// #define EEPROM_ADDR_PEEKONAME 320

void fetchIncomingImageToBuffer(const String& imageUrl) {
    if (WiFi.status() != WL_CONNECTED) {
        messageStart("WiFi"," not connected");
        return;
    }

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.begin(client, imageUrl);
    http.setTimeout(10000);

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        messageStart("HTTP"," error");
        http.end();
        return;
    }

    WiFiClient* stream = http.getStreamPtr();

    // 1️⃣ Skip +3 header bytes
    uint8_t header[3];
    size_t skipped = 0;
    unsigned long start = millis();

    while (skipped < 3 && millis() - start < 1000) {
        if (stream->available()) {
            skipped += stream->readBytes(header + skipped, 3 - skipped);
        }
        delay(1);
    }

    if (skipped != 3) {
        messageStart("Image"," header error");
        http.end();
        return;
    }

    // 2️⃣ Read exactly one frame into GLOBAL buffer
    size_t bytesRead = 0;
    start = millis();

    while (bytesRead < IMAGE_FRAME_SIZE && millis() - start < 3000) {
        if (stream->available()) {
            bytesRead += stream->readBytes(
                incomingImageBuffer + bytesRead,
                IMAGE_FRAME_SIZE - bytesRead
            );
        }
        delay(1);
    }

    http.end();

    if (bytesRead != IMAGE_FRAME_SIZE) {
        messageStart("Image"," size error");
        incomingImageReady = false;
        return;
    }

    // 3️⃣ Mark as ready (DO NOT DRAW HERE)
    incomingImageReady = true;
}




String fetchAnimationLink(const String& peekoCode) {
    if (WiFi.status() != WL_CONNECTED) return "";

    
    
    WiFiClientSecure client; // Add this
    client.setInsecure();    // Essential for Vercel/Cloudflare certs unless you use Root CA
    
    HTTPClient http;
    String url = "https://peeko-nine.vercel.app/api/v1/peeko/" + peekoCode;

    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("x-api-key", ESP32_API_KEY);

    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    // ... rest of code
    int httpCode = http.GET();
    String link = "";

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        DynamicJsonDocument doc(4096); // Taille augmentée pour les objets imbriqués
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
            // Accès au lien dans la structure JSON (à adapter selon votre modèle)
            // Exemple ici : peeko -> animationSet -> animations[0] -> link
            if (doc["peeko"]["peekoName"]) {
                displayWifi(doc["peeko"]["peekoName"]);
            }
        } else {
            Serial.print("Erreur JSON: ");
            Serial.println(error.c_str());
        }
    } else {
        Serial.printf("Erreur HTTP %d : %s\n", httpCode, http.errorToString(httpCode).c_str());
    }

    http.end();
    return link; 
}
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
void createPeeko(String peekoCode, String peekoName) {
  if (WiFi.status() != WL_CONNECTED) {
    displayWifi("WiFi not connected!");
    delay(1000);
    return;
  }
  WiFiClientSecure client;
    client.setInsecure();

  HTTPClient http;
  http.begin(client,"https://peeko-nine.vercel.app/api/v1/peeko/new"); // Replace with your server URL
  http.addHeader("Content-Type", "application/json");
  http.addHeader("x-api-key", ESP32_API_KEY);

  // Create JSON body
  StaticJsonDocument<200> doc;
  doc["code"] = peekoCode;
  if (peekoName != "") doc["peekoName"] = peekoName;
  
  String requestBody;
  serializeJson(doc, requestBody);

  int httpResponseCode = http.POST(requestBody);

  if (httpResponseCode > 0) {
  String response = http.getString();
  Serial.println("Response: " + response);

  StaticJsonDocument<200> resDoc;
  DeserializationError error = deserializeJson(resDoc, response);

  if (!error) {
    bool success = resDoc["success"] | false;

    if (success) {
    const char* firstName = resDoc["peeko"]["user"]["firstName"] | "";
    const char* lastName  = resDoc["peeko"]["user"]["lastName"]  | "";

    String displayName = "Device Registered";

    if (strlen(firstName) || strlen(lastName)) {
        displayName = String("Welcome ") + firstName + " " + lastName;
    }

    displayWifi(displayName);
    delay(2000);
    ESP.restart();
    } else {
      displayWifi("Registration Failed");
    }
  } else {
    Serial.print("JSON parse failed: ");
    Serial.println(error.c_str());
    displayWifi("Invalid Server Response");
  }

} else {
  Serial.println("Error on POST: " + String(httpResponseCode));
  displayWifi("Network Error");
}


  http.end();
}
bool saveToSpiffs(const String &url, const String &fileName) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected");
        return false;
    }

    HTTPClient http;
    http.begin(url);

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.println("Failed to download file: " + String(httpCode));
        displayWifi("Failed to download file: " + String(httpCode));
        delay(1000);
        http.end();
        return false;
    }

    WiFiClient *stream = http.getStreamPtr();
    File file = SPIFFS.open(fileName, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        displayWifi("Failed to open file for writing");
        delay(1000);
        http.end();
        return false;
    }
    displayWifi("file: " + fileName);
    delay(1000);
    
    http.writeToStream(&file);

    file.close();
    http.end();

    displayWifi("Saved to SPIFFS: " + fileName);
    delay(1000);
    return true;
}
void downloadAnimationSet(JsonArray animations) {
    for (int i = 0; i < animations.size(); i++) {
        String link = animations[i]["link"].as<String>();
        String animName = animations[i]["name"].as<String>();
        displayWifi(animName);
        // sanitize filename
        animName.replace(" ", "_");
        String filePath = "/anim_" + animName + ".bin";

        if (!SPIFFS.exists(filePath)) {
            Serial.println("Downloading: " + filePath);
            saveToSpiffs(link, filePath);
        } else {
            Serial.println("Already exists: " + filePath);
        }
    }
}

void fetchPeekoMood(const String &peekoCode, void (*callback)(String mood)) {
    if (WiFi.status() != WL_CONNECTED) {
        displayWifi("Wi-Fi not connected");
        return;
    }
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;

    String url = "https://peeko-nine.vercel.app/api/v1/peeko/" + peekoCode;
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("x-api-key", ESP32_API_KEY);

    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(10000);

    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();

        DynamicJsonDocument doc(2048);
        if (deserializeJson(doc, payload)) {
            // messageStart("JSON error");
            return;
        }

        JsonObject peeko = doc["peeko"];

        String mood = peeko["mood"] | "DEFAULT";
        String peekoName = peeko["peekoName"] | "";

        String firstName = peeko["user"]["firstName"] | "";
        String lastName  = peeko["user"]["lastName"]  | "";
        displayWifi(peekoName);
        delay(2000);
        saveUserData(firstName, lastName, peekoName);

        if (peeko.containsKey("animationSet")) {
            JsonArray animations = peeko["animationSet"]["animations"].as<JsonArray>();
            if (!animations.isNull()) {
                downloadAnimationSet(animations);
            }
        }

        playServerFrames();
        delay(2000);

        if (callback) callback(mood);
    } else {
        // messageStart("HTTP error: " + String(httpCode));
    }

    http.end();
}

void fetchIncomingMessage(const String &peekoCode) {
    if (WiFi.status() != WL_CONNECTED) {

        return;
    }

    WiFiClientSecure client;
    client.setInsecure(); // Vercel uses HTTPS

    HTTPClient http;

    String url = "https://peeko-nine.vercel.app/api/v1/message/open/" + peekoCode;
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("x-api-key", ESP32_API_KEY);
    
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.setTimeout(10000);

    int httpCode = http.sendRequest("PATCH", "{}");

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();

        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
            messageStart("JSON","parse error");
            http.end();
            return;
        }

        // Expected structure:
        // { success: true, message: { source, content, ... } }

        if (!doc["success"]) {
            // messageStart("No message");
            http.end();
            return;
        }

        JsonObject message = doc["message"];

        JsonObject userSource = message["userSource"];

        String firstName =  userSource["firstName"] | "";
        String lastName =   userSource["lastName"] | "";
        String from = firstName + " " + lastName;

        String content = message["content"] | "";

        if(message["meta"]){
            String imageUrl = message["meta"]["link"];
            fetchIncomingImageToBuffer(imageUrl);
            delay(2000);
        }

        if (from.length() > 0 && content.length() > 0) {
            messageStart(from, content);
        }

    } else {
        displayWifi("HTTP error "+String(httpCode));
    }

    http.end();
}


void updateWeather() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String url = "http://api.openweathermap.org/data/2.5/weather?q=Kelibia,TN&appid=22221fc7fc8aea0adfd9865cc2e84911&units=metric";
  
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    StaticJsonDocument<1024> doc;
    deserializeJson(doc, payload);

    currentTemp = doc["main"]["temp"];      // result: 13.14
    currentHumidity = doc["main"]["humidity"]; // result: 76
  }
  http.end();
}

