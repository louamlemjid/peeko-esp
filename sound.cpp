#include "sound.h"

Sound::Sound() {
    currentNote = 0;
    noteTimer = 0;
}

void Sound::playMelody() {
    currentNote = 0;
    noteTimer = millis();
    tone(SPEAKER_PIN, melody[currentNote]);
}

void Sound::update() {
    unsigned long now = millis();

    if (currentNote < 4) { // number of notes
        if (now - noteTimer >= noteDurations[currentNote]) {
            noTone(SPEAKER_PIN); // stop previous note

            currentNote++;
            noteTimer = now;

            if (currentNote < 4) {
                tone(SPEAKER_PIN, melody[currentNote]);
            }
        }
    }
}
