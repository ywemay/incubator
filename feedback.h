#ifndef __FEEDBACK_H__
#define __FEEDBACK_H__

#include <Arduino.h>
#include "beeper.h"
#ifdef OLED_ON
#include "oled.h"
#endif

#ifdef LEDS_ON
#include "leds.h"
#endif

class Feedback {
  
  public:
    void setup();
    void sensor_ok();
    void sensor_setup_fail();
    void setup_ok();
    void error();
    void display_sensor(int8_t state);
    void stats(float t, float h, unsigned int turning, unsigned int remained, int8_t state);
    void restoreSensor();
    void restarting();
};

#endif