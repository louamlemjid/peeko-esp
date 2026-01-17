#pragma once
#include <Adafruit_SSD1306.h>
#include <FluxGarage_RoboEyes.h>
#undef N
// Only forward declare RoboEyes object, don't include library globals in multiple files
extern Adafruit_SSD1306 display;
extern RoboEyes<Adafruit_SSD1306> eyes;

void displayInit();
void displayUpdate();
void drawMessageIcon();
void displayPrepareTextAuto(const String& text);
void displayTypeChar(char c);
void clearDisplay();
void moodUpdate(String mood);
void moodToggle();
void animationMood();
void displayClock();
void displayWelcome();
void displayWifi(const String& text);
void displayGreeting(String firstName, String lastName, String peekoName);
void updateMenu();
void displayMenu();
void displayWifiIcon();
void playSpiffsAnimations();
void displayWeather();
void displayPeekoDoro();
void displayIncomigMessage(const String& from,const String& content);
void drawIncomingImageIfAny();