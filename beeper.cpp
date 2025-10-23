#include "beeper.h"

void setup_beeper() {
  pinMode(BEEPER_PIN, OUTPUT);
  digitalWrite(BEEPER_PIN, LOW);
}

void beep(unsigned long sec, unsigned long pause, unsigned int repeat) {
  unsigned long duration = sec > 0 ? sec : 500;
  for (int i = 0; i <= 1; i++) {
    digitalWrite(BEEPER_PIN, HIGH);
    delay(duration);
    digitalWrite(BEEPER_PIN, LOW);
    if (pause) delay(pause);
  }  
}