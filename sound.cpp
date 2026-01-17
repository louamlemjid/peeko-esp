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
void Sound::reset() {
    currentNote = 0;
    noTone(SPEAKER_PIN);
}
void Sound::update() {
    unsigned long now = millis();

    if (currentNote < 24) { // number of notes
        if (now - noteTimer >= 70) {
            noTone(SPEAKER_PIN); // stop previous note

            currentNote++;
            noteTimer = now;

            if (currentNote < 24) {
                tone(SPEAKER_PIN, melody[currentNote]);
            }
        }
    }
}
