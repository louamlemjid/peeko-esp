#pragma once
#include <Arduino.h>

// Initialize touch sensor
void touchBegin(int pin);

// Update touch sensor, call doSomething() on trigger
void touchUpdate();

// Set what function to run on touch
void touchSetCallback(void (*cb)());

// Pin the sensor is connected to
extern int touchPin;

