#pragma once
// TouchSensor.h
#ifndef TOUCH_SENSOR_H
#define TOUCH_SENSOR_H

#include <Arduino.h>
#include "app_state.h"

class TouchSensor {
  protected:
    int _pin;
    bool _lastState;
    unsigned long _pressStartTime;
    unsigned long _lastTriggerTime;
    const unsigned long _cooldown;
    const unsigned long _longPressThreshold = 800; // 0.8 seconds for a long press

  public:
    TouchSensor(int pin, unsigned long cooldown = 200) 
        : _pin(pin), _lastState(false), _pressStartTime(0), 
          _lastTriggerTime(0), _cooldown(cooldown) {}
      
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

        // 1. Detection: Finger just touched the sensor (Rising Edge)
        if (currentState == HIGH && !_lastState) {
            if (now - _lastTriggerTime > _cooldown) {
                _pressStartTime = now; 
            }
        }

        // 2. Detection: Finger just released the sensor (Falling Edge)
        if (currentState == LOW && _lastState) {
            unsigned long pressDuration = now - _pressStartTime;

            if (pressDuration > 50) { // Basic noise debouncing
                if (pressDuration >= _longPressThreshold) {
                    longPress();
                } else {
                    oneTouch();
                }
                _lastTriggerTime = now;
            }
        }

        _lastState = currentState;
    }
};

#endif