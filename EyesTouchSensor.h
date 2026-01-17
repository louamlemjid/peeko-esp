#ifndef EYES_TOUCH_SENSOR_H
#define EYES_TOUCH_SENSOR_H

#include "touchsensor.h"
#include "display.h"
#include "app_state.h"
#include "peekoDoro.h"

extern const char* menuItems[];
extern int menuIndex;
extern bool skipAnimation;
extern const int menuCount;
extern PeekoDoro peekoDoro;
extern MessageState msgState;

class PeekoMoodSensor : public TouchSensor {
public:
  PeekoMoodSensor(int pin)
    : TouchSensor(pin) {}

  void oneTouch() override {
    moodToggle();
  }

  // void longPress() override {
  //     Serial.println("Action: Reset Peeko to Factory");
  //     // resetDevice();
  // }
};
class MenuTouchSensor : public TouchSensor {
public:
  MenuTouchSensor(int pin)
    : TouchSensor(pin) {}

  void longPress() override {
    // menuIndex is likely a global variable from your menu logic
    const char* item = menuItems[menuIndex];

    // We use String() to make comparison easy
    String itemStr = String(item);

    if (itemStr == "Animation") {
      enterMode(MODE_ANIMATION);
    } else if (itemStr == "Clock") {
      enterMode(MODE_CLOCK);
    } else if (itemStr == "Eyes") {
      enterMode(MODE_EYES);
    } else if (itemStr == "Wifi") {
      enterMode(MODE_WIFI);
    } else if (itemStr == "Weather") {
      enterMode(MODE_WEATHER);
    } else if (itemStr == "PeekoDoro") {
      enterMode(MODE_PEEKODORO);
    }
  }
  void oneTouch() override {
    menuIndex = (menuIndex + 1) % menuCount;
  }
};
class AnimationTouchSensor : public TouchSensor {
public:
  AnimationTouchSensor(int pin)
    : TouchSensor(pin) {}

  void oneTouch() override {
    skipAnimation = true;
  }
};

class ClockTouchSensor : public TouchSensor {
public:
  ClockTouchSensor(int pin)
    : TouchSensor(pin) {}

  void oneTouch() override {
    enterMode(MODE_WEATHER);
  }
};

class WeatherTouchSensor : public TouchSensor {
public:
  WeatherTouchSensor(int pin)
    : TouchSensor(pin) {}

  void oneTouch() override {
    enterMode(MODE_CLOCK);
  }
};

class PeekoDoroTouchSensor : public TouchSensor {
public:
  PeekoDoroTouchSensor(int pin)
    : TouchSensor(pin) {}

  void oneTouch() override {
    peekoDoro.toggleStop();
  }
};

class MessageTouchSensor : public TouchSensor {
public:
  MessageTouchSensor(int pin)
    : TouchSensor(pin) {}

  void oneTouch() override {
    if(msgState == MSG_IDLE) enterMode(MODE_EYES);
  }
};
#endif