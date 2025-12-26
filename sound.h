#ifndef SOUND_H
#define SOUND_H

#include <Arduino.h>

// Pin connected to the speaker
#define SPEAKER_PIN 2

class Sound {
public:
    Sound();                 // constructor
    void playMelody();       // start melody
    void update();           // call in loop to progress notes

private:
    // HMM sound: two tones sliding slightly
    int melody[4] = {330, 349, 330, 349};       
    int noteDurations[4] = {300, 300, 300, 300};
    int currentNote;
    unsigned long noteTimer;
};

#endif
