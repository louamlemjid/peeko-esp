#pragma once
#include <Arduino.h>

class Battery {
public:
  Battery(int pin) : _pin(pin) {}

  void begin() {
    pinMode(_pin, INPUT);
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    _filtered = analogRead(_pin); // initial value
  }

  void update() {
    int raw = analogRead(_pin);

    // Low-pass filter (α = 0.1)
    _filtered = _filtered + 0.1f * (raw - _filtered);

    float voltage = (_filtered / 4095.0f) * 3.3f * 2.0f;

    if (voltage >= 4.0)      _batteryLevel = 4;
    else if (voltage >= 3.8) _batteryLevel = 3;
    else if (voltage >= 3.6) _batteryLevel = 2;
    else if (voltage >= 3.4) _batteryLevel = 1;
    else                     _batteryLevel = 0;
  }

  int getBatteryLevel() const {
    return _batteryLevel;
  }

private:
  int _pin;
  int _batteryLevel = 0;
  float _filtered = 0;
};
