#include "display.h"
#include <Wire.h>
#include <time.h>
#include "welcome.h"
#include "wifiicon.h"
#include <FS.h>
#include <SPIFFS.h>
#include "fetch.h"
#include "peekoDoro.h"
#include "app_state.h"

extern uint8_t incomingImageBuffer[IMAGE_FRAME_SIZE];
extern bool incomingImageReady;

extern PeekoDoro peekoDoro;
bool blinkColon = true;

const char* menuItems[] = {
    " ",
  "Animation",
  "Clock",
  "Weather",
  "PeekoDoro",
  "Eyes",
  "Wifi"
};
bool skipAnimation = false;

int menuCount = sizeof(menuItems) / sizeof(menuItems[0]);
int menuIndex = 0;                 // currently selected (BIG)
unsigned long menuTimer = 0;

float currentTemp = 0.0;
int currentHumidity = 0;
unsigned long lastWeatherUpdate = 0;
const unsigned long weatherInterval = 600000; // Update every 10 minutes

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
// These stay in memory between function calls
static File rootDir;
static File currentAnimFile;
static int currentFrame = 0;
static unsigned long fileStartTime = 0;
static unsigned long lastFrameMillis = 0;

void playSpiffsAnimations() {
    unsigned long now = millis();

    // 1. Check for Skip Request or Timeout (5 seconds)
    if (skipAnimation || (currentAnimFile && (now - fileStartTime > 5000))) {
        if (currentAnimFile) currentAnimFile.close();
        currentAnimFile = File(); // Reset file object
        skipAnimation = false;    // Reset skip flag
        currentFrame = 0;
        // Proceed to next file logic below
    }

    // 2. If no file is open, find the next valid "anim_" file
    if (!currentAnimFile) {
        if (!rootDir) rootDir = SPIFFS.open("/");
        
        File nextFile = rootDir.openNextFile();
        
        // If we reached the end of SPIFFS, start over
        if (!nextFile) {
            rootDir.rewindDirectory();
            return; 
        }

        String name = nextFile.name();
        if (name.startsWith("/anim_") || name.startsWith("anim_")) {
            currentAnimFile = nextFile;
            fileStartTime = now;
            currentFrame = 0;
        } else {
            nextFile.close();
            return; // Skip non-animation files
        }
    }

    // 3. Frame Rate Control (Draw 1 frame every 70ms)
    if (now - lastFrameMillis >= 90) {
        lastFrameMillis = now;

        int fileSize = currentAnimFile.size();
        int totalFrames = fileSize / 1024;
        if (totalFrames == 0) return;

        float realStep = (float)fileSize / (float)totalFrames;
        
        // Calculate position with your +3 byte nudge
        long frameStart = (long)(currentFrame * realStep) + 3;
        currentAnimFile.seek(frameStart);

        uint8_t frameBuffer[1024];
        if (currentAnimFile.read(frameBuffer, 1024) == 1024) {
            display.clearDisplay();
            display.drawBitmap(0, 0, frameBuffer, 128, 64, SSD1306_WHITE);
            display.display();
        }

        // Advance frame counter
        currentFrame++;
        if (currentFrame >= totalFrames) {
            currentFrame = 0; // Loop the same file until 5s is up or skip is pressed
        }
    }
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

  char timeBuf[6];
  sprintf(timeBuf, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);

  char tempBuf[24];
  sprintf(tempBuf, "%02d / %02d / %02d", timeinfo.tm_mday,timeinfo.tm_mon + 1,timeinfo.tm_year + 1900);

  display.clearDisplay();

  // ---- TIME (BIG) ----
  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(22, 5);
  display.print(timeBuf);

  // ---- date (SMALL) ----
  display.setTextSize(1.8);
  display.setCursor(22, 48);
  display.print(tempBuf);

  display.display();
}

void displayPeekoDoro() {

  char workBuf[20];
  sprintf(workBuf, "Work  %02d:%02d",peekoDoro.getWorkPeriod() / 60 , peekoDoro.getWorkPeriod() % 60);

  char breakBuf[24];
  sprintf(breakBuf, "Break  %02d:%02d", peekoDoro.getBreakPeriod() / 60,peekoDoro.getBreakPeriod() % 60);

    char setsBuf[5];
    sprintf(setsBuf,"#%02d", peekoDoro.getSets());

  display.clearDisplay();


  if(peekoDoro.getWorkPeriod() > 0){
     // ---- TIME (BIG) ----
  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(22, 0);
  display.print(workBuf);

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(105,10);
    display.print(setsBuf);

  // ---- date (SMALL) ----
  display.setTextSize(1.8);
  display.setCursor(25, 55);
  display.print(breakBuf);

  }
  else{
    display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 0);
  display.print(breakBuf);

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(105,10);
    display.print(setsBuf);

  // ---- date (SMALL) ----
  display.setTextSize(1.8);
  display.setCursor(25, 55);
  display.print(workBuf);
  }
 

  display.display();
}

void displayWeather() {
  // Update weather only if interval has passed
  if (millis() - lastWeatherUpdate > weatherInterval || lastWeatherUpdate == 0) {
    updateWeather();
    lastWeatherUpdate = millis();
  }

  char tempBuf[10];
  char humBuf[10];
  
  // Format strings: "13.1 C" and "76 %"
  dtostrf(currentTemp, 2, 1, tempBuf); 
  sprintf(humBuf, "%d %%", currentHumidity);

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // ---- TEMPERATURE (BIG) ----
  display.setTextSize(3);
  display.setCursor(20, 5);
  display.print(tempBuf);
  display.setTextSize(1);
  display.print(" C");

  // ---- HUMIDITY (SMALL) ----
  display.setTextSize(2);
  display.setCursor(35, 45);
  display.print(humBuf);
  
  // Draw a small icon or label
  display.setTextSize(1);
  display.setCursor(85, 48);
  display.print("HUM");

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

    eyes.setMood(moodValue); 
    // Serial.print("Mood set to: ");
    // Serial.println(selectedMood);
    eyes.update();
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

void displayIncomigMessage(const String& from,const String& content){
    char fromBuf[20];
    from.toCharArray(fromBuf, sizeof(fromBuf));

    char bodyBuf[50];
    content.toCharArray(bodyBuf, sizeof(bodyBuf));

    display.clearDisplay();

    // ==== sender====
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(5, 5);
    display.print(fromBuf);
    display.print(" :");

    // ==== body ====
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(5, 15);
    display.print(bodyBuf);

    display.drawRect(0, 0, 128, 64, SSD1306_WHITE);

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
    // unsigned long now = millis();

    // if (now - menuTimer >= 3000) {
    //     menuTimer = now;
    //     menuIndex = (menuIndex + 1) % menuCount;
        displayMenu();
    // }
}

void drawIncomingImageIfAny() {
    if (!incomingImageReady) return; // global flag

    display.clearDisplay();
    display.drawBitmap(
        0,
        0,
        incomingImageBuffer, // global buffer
        128,
        64,
        SSD1306_WHITE
    );
    display.display();
}



void displayTypeChar(char c) {
    display.print(c);
    display.display();
}

void clearDisplay() {
    display.clearDisplay();
    display.display();
}
