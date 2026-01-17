// #include "touchsensor.h"
// #include "display.h"
// #include "app_state.h"
// #include "sound.h"

// extern Sound peekoSound;
// int touchPin;
// static bool wasTouched = false;

// // Initialize touch sensor
// void touchBegin(int pin) {
//     touchPin = pin;
//     pinMode(touchPin, INPUT);
// }

// // The action triggered when the sensor is touched
//  void touchAction() {
//     // launchMenu();
//     // toggleMode();
//     peekoSound.reset();
// }

// static unsigned long lastTriggerTime = 0;
// const unsigned long TOUCH_COOLDOWN_MS = 200; // Prevent retrigger within 200ms

// void touchUpdate() {
//     if(digitalRead(touchPin) == HIGH){
    
//         touchAction();
//     }

// }

