#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "beeper.h"

class Oled {
  
  public:
    // Oled() {};
    void setup();
    void display_sensor(int8_t state);
    void stats(float t, float h, unsigned int turning, unsigned int remained);
};

#endif