#include "app_state.h"
#include "display.h"
#include "animation.h"

MessageState msgState = MSG_IDLE;
String messageText = "";
unsigned long msgTimer = 0;
int msgIndex = 0;

DisplayMode currentMode = MODE_EYES;
unsigned long stateTimer = 0;

void enterMode(DisplayMode mode) {
    currentMode = mode;
    stateTimer = millis();
}

void messageStart(const String& text) {
  messageText = text;
  msgState = MSG_ICON;
  msgIndex = 0;
  currentMode = MODE_MESSAGE;
  stateTimer = millis();
}

void launchMenu(){
  currentMode = MODE_MENU;
  stateTimer = millis();
}
void toggleMode() {
  currentMode = (DisplayMode)((currentMode + 1) % 6);

  if (currentMode == MODE_MESSAGE) {
    currentMode = MODE_EYES;
  }
  if (currentMode == MODE_MENU) {
    currentMode = MODE_ANIMATION;
  }

  stateTimer = millis();
}

void loopDisplay() {
  unsigned long now = millis();

  switch (currentMode) {
    case MODE_ANIMATION:
      playFrames(); // loop through frames
      if (now - stateTimer > 15000) { // 4 seconds
          stateTimer = now;
          currentMode = MODE_CLOCK;
      }
      break;

    case MODE_MESSAGE:
      messageUpdate(); // handles typing & MSG_WAIT
      if (msgState == MSG_IDLE) {
        currentMode = MODE_EYES;
        stateTimer = now;
      }
      break;

    case MODE_EYES:
      displayUpdate();

      if (now - stateTimer >= 28000) {
          currentMode = MODE_CLOCK;
          stateTimer = now;
      }
      break;


    case MODE_CLOCK:
      if (now - stateTimer >= 1000) {
          displayClock();
      }

      if (now - stateTimer >= 8000) {
          currentMode = MODE_ANIMATION;
          stateTimer = now;
      }
      break;

    case MODE_MENU:
      updateMenu();
      break;
  }
}

void messageUpdate() {
  unsigned long now = millis();

  switch (msgState) {
    case MSG_IDLE: break;

    case MSG_ICON:
      drawMessageIcon();
      if (now - stateTimer > 1000) {
        msgState = MSG_TYPING;
        stateTimer = now;
        displayPrepareTextAuto(messageText);
      }
      break;

    case MSG_TYPING:
      if (msgIndex < messageText.length()) {
        displayTypeChar(messageText[msgIndex++]);
      } else {
        msgState = MSG_WAIT;
        stateTimer = now;
      }
      break;

    case MSG_WAIT:
      if (now - stateTimer > 4000) {
        msgState = MSG_IDLE;
      }
      break;
  }
}
