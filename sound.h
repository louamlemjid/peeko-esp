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
    void reset(); 

private:
    // HMM sound: two tones sliding slightly
    int melody[24] = {128,130,133,136,138,140,141,142,142,141,140,138,136,133,130,128,
        125,122,119,117,115,114,113,113};       
    int noteDurations[4] = {50, 50, 50, 50};
    int currentNote;
    unsigned long noteTimer;
};

#endif
