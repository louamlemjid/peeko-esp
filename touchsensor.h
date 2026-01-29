#pragma once
// TouchSensor.h
#ifndef TOUCH_SENSOR_H
#define TOUCH_SENSOR_H

#include <Arduino.h>
#include "app_state.h"
#include "display.h"

class TouchSensor {
  protected:
    int _pin;
    bool _lastState = false;
    bool _longPressFired = false;
    unsigned long _pressStartTime = 0;
    unsigned long _longPressThreshold = 800;
    const unsigned long _cooldown;

  public:
    TouchSensor(int pin, unsigned long cooldown = 200) 
        : _pin(pin), _lastState(false), _pressStartTime(0), 
           _cooldown(cooldown) {}
      
    virtual void begin() {
        pinMode(_pin, INPUT);
    }

    // Pure virtual functions: Must be implemented by subclasses
    virtual void oneTouch() = 0;
    virtual void longPress(){
      launchMenu();
    }

   void update() {
    bool currentState = digitalRead(_pin);
    unsigned long now = millis();

    // 1️⃣ Touch started
    if (currentState == HIGH && !_lastState) {
        _pressStartTime = now;
        _longPressFired = false;
    }

    // 2️⃣ Touch still active → check long press
    if (currentState == HIGH && !_longPressFired) {
        if (now - _pressStartTime >= _longPressThreshold) {
            displayLongTouch();
            longPress();                 // 🔥 fires at 800ms
            _longPressFired = true;      // prevent repeat
        }
    }

    // 3️⃣ Touch released
    if (currentState == LOW && _lastState) {
        unsigned long pressDuration = now - _pressStartTime;

        // Only short press if long press didn't fire
        if (!_longPressFired && pressDuration > 50) {
            oneTouch();
            displayTouch();
        }
    }

    _lastState = currentState;
}

};

#endif