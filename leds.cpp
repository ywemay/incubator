#include "leds.h"

void Leds::setup() {
  pinMode(LED_COLD, OUTPUT);
  pinMode(LED_OK, OUTPUT);
  pinMode(LED_HOT, OUTPUT);
  off();
}

void Leds::cold(uint8_t mode) {
  digitalWrite(LED_COLD, mode);
}

void Leds::ok(uint8_t mode) {
  digitalWrite(LED_OK, mode);
}

void Leds::hot(uint8_t mode) {
  digitalWrite(LED_HOT, mode);
}

void Leds::all(uint8_t mode) {
  hot(mode);
  ok(mode);
  cold(mode);
}

void Leds::beep(uint8_t pin, unsigned long sec, unsigned long pause, unsigned int repeat) {
  unsigned long duration = sec > 0 ? sec : 500;
  for (int i = 0; i <= 1; i++) {
    digitalWrite(pin, ON);
    beeper_on();
    delay(duration);
    digitalWrite(pin, OFF);
    beeper_off();
    if (pause) delay(pause);
  }  
}

void Leds::hot_beep(unsigned long sec, unsigned long pause, unsigned int repeat) {
  beep(LED_HOT, sec, pause, repeat);
}