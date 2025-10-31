#ifndef __LEDS_H__
#define __LEDS_H__

#include <Arduino.h>
#include "beeper.h"

#define ON LOW
#define OFF HIGH

class Leds {
  public:
    void setup();
    void cold(uint8_t mode);
    void ok(uint8_t mode);
    void hot(uint8_t mode);
    void all(uint8_t mode);
    void off() { all(OFF); };
    void hot_beep(unsigned long sec = 0, unsigned long pause = 0, unsigned int repeat = 1);
    // void ok_beep(unsigned long sec = 0, unsigned long pause = 0, unsigned int repeat = 1);
    // void cold_beep(unsigned long sec = 0, unsigned long pause = 0, unsigned int repeat = 1);
  private:
    void beep(uint8_t pin, unsigned long sec = 0, unsigned long pause = 0, unsigned int repeat = 1);
};

#endif