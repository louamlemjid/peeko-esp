#include "display.h"
#include <Wire.h>
#include <time.h>
#include "welcome.h"
#include "wifiicon.h"
bool blinkColon = true;

const char* menuItems[] = {
    " ",
  "Animation",
  "Clock",
  "PeekoDoro",
  "Eyes"
};

const int menuCount = sizeof(menuItems) / sizeof(menuItems[0]);
int menuIndex = 0;                 // currently selected (BIG)
unsigned long menuTimer = 0;


// Define global objects here **only once**
Adafruit_SSD1306 display(128, 64, &Wire, -1);
RoboEyes<Adafruit_SSD1306> eyes(display);

uint8_t moodValue = DEFAULT; 
 
static const uint8_t moods[] = {DEFAULT, HAPPY, TIRED, ANGRY};
static const char* moodNames[] = {"DEFAULT", "HAPPY", "TIRED", "ANGRY"};
static const uint8_t moodsCount = sizeof(moods) / sizeof(moods[0]);

void displayInit() {
    Wire.begin(20, 21);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    eyes.begin(128, 64, 60);
    eyes.setMood(DEFAULT);
    eyes.setCuriosity(true);
    eyes.anim_laugh();
    eyes.setAutoblinker(true, 4, 0);
    eyes.setIdleMode(true, 3, 2);
}

void displayUpdate() {
    eyes.update();
}
void animationMood(){
    eyes.setMood(HAPPY);
    eyes.anim_laugh();
    eyes.anim_laugh();
    eyes.update();
}
void moodToggle() {
    // Find current index
    uint8_t index = 0;
    for (uint8_t i = 0; i < moodsCount; i++) {
        if (moods[i] == moodValue) {
            index = i;
            break;
        }
    }

    // Move to next mood
    index = (index + 1) % moodsCount;
    moodValue = moods[index];

    // Apply mood
    eyes.setMood(moodValue);
    eyes.update();

    // Print to serial
    Serial.print("Mood toggled to: ");
    Serial.println(moodNames[index]);
}
void displayClock() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  // Example temperature (replace with real sensor value)
  int temperature = 26;

  char timeBuf[6];
  sprintf(timeBuf, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);

  char tempBuf[24];
  sprintf(tempBuf, "Temperature: %.1d C", temperature);

  display.clearDisplay();

  // ---- TIME (BIG) ----
  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(15, 5);
  display.print(timeBuf);

  // ---- TEMPERATURE (SMALL) ----
  display.setTextSize(1.8);
  display.setCursor(0, 48);
  display.print(tempBuf);

  display.display();
}

void displayWifiIcon(){
    display.clearDisplay();

    display.drawBitmap(0, 0, wifiIcon , 128, 64, SSD1306_WHITE);
    
    display.display();
}

void moodUpdate(String mood){
  String selectedMood = mood;
    

    if (selectedMood == "DEFAULT") moodValue = DEFAULT;
    else if (selectedMood == "HAPPY") moodValue = HAPPY;
    else if (selectedMood == "TIRED") moodValue = TIRED;
    else if (selectedMood == "ANGRY") moodValue = ANGRY;

    // eyes.setMood(moodValue); 
    // Serial.print("Mood set to: ");
    // Serial.println(selectedMood);
    // eyes.update();
}

void drawMessageIcon() {
    display.clearDisplay();
    display.drawRect(28, 18, 72, 36, SSD1306_WHITE);
    display.drawLine(28, 18, 64, 36, SSD1306_WHITE);
    display.drawLine(100, 18, 64, 36, SSD1306_WHITE);
    display.display();
}

void displayPrepareTextAuto(const String& text) {
    display.clearDisplay();
    int len = text.length();

    // Choose text size
    int textSize;
    if (len <= 8) textSize = 3;
    else if (len <= 24) textSize = 2;
    else textSize = 1;

    display.setTextSize(textSize);
    display.setTextColor(SSD1306_WHITE);

    int x, y;

    if (len > 24) {
        // Long text: start top-left
        x = 0;
        y = 0;
    } else {
        // Short/medium text: center it
        int textWidth = len * 6 * textSize; // each character ~6px wide
        x = (128 - textWidth) / 2;
        if (x < 0) x = 0;
        y = (64 - 8 * textSize) / 2; // vertical centering, 8px per character height
    }

    display.setCursor(x, y);
}

void displayWelcome(){
    display.clearDisplay();

    display.drawBitmap(0, 0, welcome , 128, 64, SSD1306_WHITE);
    
    display.display();
}

void displayWifi(const String& text){
    char wifiBuf[50];
    text.toCharArray(wifiBuf, sizeof(wifiBuf)); // convert String to char array

    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 5);
    display.print(wifiBuf);

    display.display();
}

void displayGreeting(String firstName, String lastName, String peekoName) {

    // Make local uppercase copies (safe)
    String fn = firstName;
    String ln = lastName;
    String pn = peekoName;

    fn.toUpperCase();
    ln.toUpperCase();
    pn.toUpperCase();

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    // ---- Line 1 ----
    display.setTextSize(1);
    display.setCursor(0, 5);
    display.print("Aslema ");
    display.print(fn);
    display.print(" ");
    display.print(ln);

    // ---- Line 2 ----
    display.setCursor(0, 30);
    display.print("Ana Sahbek ");

    // ---- Line 3 ----
    display.setCursor(0, 45);

    display.print(pn);


    display.display();
}

void displayMenu() {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    int prev = (menuIndex - 1 + menuCount) % menuCount;
    int next = (menuIndex + 1) % menuCount;

    // ---- TOP (small, size 1, centered) ----
    display.setTextSize(1);
    int topX = (128 - strlen(menuItems[prev]) * 6 * 1) / 2;
    if (topX < 0) topX = 0;
    display.setCursor(topX, 5);
    display.print(menuItems[prev]);

    // ---- MIDDLE (big, size 2, centered) ----
    display.setTextSize(2);
    int middleX = (128 - strlen(menuItems[menuIndex]) * 6 * 2) / 2;
    if (middleX < 0) middleX = 0;
    display.setCursor(middleX, 25);
    display.print(menuItems[menuIndex]);

    // ---- BOTTOM (small, size 1, centered) ----
    display.setTextSize(1);
    int bottomX = (128 - strlen(menuItems[next]) * 6 * 1) / 2;
    if (bottomX < 0) bottomX = 0;
    display.setCursor(bottomX, 55);
    display.print(menuItems[next]);

    display.display();
}


void updateMenu() {
    unsigned long now = millis();

    if (now - menuTimer >= 3000) {
        menuTimer = now;
        menuIndex = (menuIndex + 1) % menuCount;
        displayMenu();
    }
}

void displayTypeChar(char c) {
    display.print(c);
    display.display();
}

void clearDisplay() {
    display.clearDisplay();
    display.display();
}
