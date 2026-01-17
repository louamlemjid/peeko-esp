#include "server.h"
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include "serveranimation.h"

extern Adafruit_SSD1306 display;

const unsigned long FRAME_DURATION = 70; // ms per frame

void playServerFrames() {
    
      for (int i = 0; i < server_allArray_LEN; i++) {
        display.clearDisplay();
        display.drawBitmap(0, 0, server_allArray[i], 128, 64, SSD1306_WHITE);
        display.display();
        delay(FRAME_DURATION); // simple pause between frames
    
    }
}