#ifndef __FEEDBACK_H__
#define __FEEDBACK_H__

#include <Arduino.h>
#include "beeper.h"
#include "config.h"
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
    void displayIP(const String& ip_address);
    void setIPAddress(const String& ip_address);
    String getIPAddress() const;
    
    // New display methods
    void displayIncubatorInfo(float current_temp, float current_humidity, 
                             float target_temp, unsigned int turn_interval,
                             unsigned int incubation_day, unsigned int total_days,
                             bool wifi_connected, unsigned int turning = 0, 
                             unsigned int remained = 0);
};

#endif