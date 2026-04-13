#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "beeper.h"

class Oled {
  private:
    bool display_available;
    
  public:
    Oled() : display_available(false) {};
    bool setup();
    void display_sensor(int8_t state);
    void stats(float t, float h, unsigned int turning, unsigned int remained, int8_t state);
    void restoreSensor();
    void restarting();
    void displayIP(const String& ip_address);
    
    // Check if display is available
    bool isAvailable() const { return display_available; }
};

#endif