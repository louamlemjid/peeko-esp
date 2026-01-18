#pragma once
#include <Arduino.h>
#define IMAGE_FRAME_SIZE 1024

enum MessageState {
  MSG_IDLE,
  MSG_ICON,
  MSG_TYPING,
  MSG_WAIT,
  MSG_IMAGE
};

enum DisplayMode {
  MODE_ANIMATION,
  MODE_MESSAGE,
  MODE_EYES,
  MODE_CLOCK,
  MODE_WEATHER,
  MODE_MENU,
  MODE_WIFI,
  MODE_PEEKODORO,
  MODE_ABOUT
};

extern MessageState msgState;
extern String messageText;
extern unsigned long msgTimer;
extern int msgIndex;
extern DisplayMode currentMode;
extern unsigned long stateTimer;

void messageStart(const String& from, const String& content);
void messageUpdate();
void loopDisplay();
void enterMode(DisplayMode mode);
void launchMenu();
void toggleMode();