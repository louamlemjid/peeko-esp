#pragma once
#include <Arduino.h>

enum MessageState {
  MSG_IDLE,
  MSG_ICON,
  MSG_TYPING,
  MSG_WAIT
};

enum DisplayMode {
  MODE_ANIMATION,
  MODE_MESSAGE,
  MODE_EYES,
  MODE_CLOCK,
  MODE_MENU
};

extern MessageState msgState;
extern String messageText;
extern unsigned long msgTimer;
extern int msgIndex;
extern DisplayMode currentMode;
extern unsigned long stateTimer;

void messageStart(const String& text);
void messageUpdate();
void loopDisplay();
void enterMode(DisplayMode mode);
void launchMenu();
void toggleMode();