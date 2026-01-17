#include <WiFi.h>
#include "app_state.h"
#include "display.h"
#include "animation.h"
#include "EyesTouchSensor.h"
#include "web.h"

extern PeekoMoodSensor peekoMoodSensor;
extern MenuTouchSensor menuTouchSensor;
extern AnimationTouchSensor animationTouchSensor;
extern ClockTouchSensor clockTouchSensor;
extern WeatherTouchSensor weatherTouchSensor;
extern PeekoDoroTouchSensor peekoDoroTouchSensor;
extern MessageTouchSensor messageTouchSensor;

MessageState msgState = MSG_IDLE;
String messageFrom = "";
String messageContent = "";
unsigned long msgTimer = 0;
int msgIndex = 0;

DisplayMode currentMode = MODE_ANIMATION;
unsigned long stateTimer = 0;

void enterMode(DisplayMode mode) {
    currentMode = mode;
    stateTimer = millis();
}

//==== for image buffer =====
#define IMAGE_FRAME_SIZE 1024
uint8_t incomingImageBuffer[IMAGE_FRAME_SIZE];
bool incomingImageReady = false;




void messageStart(const String& from, const String& content) {
    messageFrom = from;
    messageContent = content;
    msgState = MSG_ICON; // start state
    msgIndex = 0;
    currentMode = MODE_MESSAGE;
    stateTimer = millis();

    // If image is ready, mark a special state
    if (incomingImageReady) {
        msgState = MSG_IMAGE; // define a new state
    }
}


void launchMenu(){
  currentMode = MODE_MENU;
  stateTimer = millis();
}
void toggleMode() {
  currentMode = (DisplayMode)((currentMode + 1) % 5);

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
      // playFrames(); // loop through frames
      playSpiffsAnimations();
      animationTouchSensor.update();
      // if (now - stateTimer > 10000) { // 4 seconds
      //     stateTimer = now;
      //     // currentMode = MODE_EYES;
      // }
      break;

    case MODE_MESSAGE:
      messageUpdate(); // handles typing & MSG_WAIT
      messageTouchSensor.update();
      break;

    case MODE_EYES:
      displayUpdate();
      peekoMoodSensor.update();

      // if (now - stateTimer >= 15000) {
      //     currentMode = MODE_CLOCK;
      //     stateTimer = now;
      // }
      break;


    case MODE_CLOCK:
      clockTouchSensor.update();
      if (now - stateTimer >= 1000) {
        if (WiFi.status() != WL_CONNECTED) {

          currentMode = MODE_EYES;
        }
          displayClock();
      }

      // if (now - stateTimer >= 10000) {
      //     currentMode = MODE_ANIMATION;
      //     stateTimer = now;
      // }
      break;

    case MODE_WEATHER:
      weatherTouchSensor.update();
      if (now - stateTimer >= 1000) {
        if (WiFi.status() != WL_CONNECTED) {

          currentMode = MODE_EYES;
        }
          displayWeather();
      }
      
      break;

    case MODE_MENU:
      updateMenu();
      menuTouchSensor.update();
      break;

    case MODE_PEEKODORO:
      displayPeekoDoro();
      peekoDoroTouchSensor.update();
      break;

    case MODE_WIFI:
      WiFi.softAP("Peeko");
      webInit();
      currentMode = MODE_EYES;
          stateTimer = now;
      break;
  }
}

void messageUpdate() {
  unsigned long now = millis();

  switch (msgState) {
    case MSG_IDLE: break;

    case MSG_ICON:
      drawMessageIcon();
      if (now - stateTimer > 2000) {
        msgState = MSG_TYPING;
        stateTimer = now;
        // displayPrepareTextAuto(messageText);
      }
      break;

    case MSG_IMAGE:
      drawIncomingImageIfAny();
      // Automatically go to MSG_TYPING after 2 seconds
      if (now - stateTimer > 4000) {
          msgState = MSG_TYPING;
          stateTimer = now;
          incomingImageReady = false; // clear flag
      }
      break;

    case MSG_TYPING:
      // if (msgIndex < messageText.length()) {
      //   displayTypeChar(messageText[msgIndex++]);
      // } else {
      //   msgState = MSG_WAIT;
      //   stateTimer = now;
      // }
      displayIncomigMessage(messageFrom,messageContent);
      msgState = MSG_WAIT;
        stateTimer = now;
      break;

    case MSG_WAIT:
      if (now - stateTimer > 4000) {
        msgState = MSG_IDLE;
      }
      break;
  }
}
