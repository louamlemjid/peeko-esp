#include <Arduino.h>
#include "animation.h"
#include "frames.h"
#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 display;

const unsigned long FRAME_DURATION = 70; // ms per frame

void playFrames() {
    for (int i = 0; i < epd_bitmap_allArray_LEN; i++) {
        display.clearDisplay();
        display.drawBitmap(0, 0, epd_bitmap_allArray[i], 128, 64, SSD1306_WHITE);
        display.display();
        delay(FRAME_DURATION); // simple pause between frames
    }
}
