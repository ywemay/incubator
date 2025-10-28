#ifndef __TURNER_H__
#define __TURNER_H__

#include <Arduino.h>
#include "config.h"

#ifdef EGGS_TURNER_SERVO_PIN
#include <ESP32Servo.h>
#endif

class Turner {

  public:
    // Turner() {};
    void setup();
    unsigned int process();
    void turn();
    void stop();
    unsigned int remained();
};

#endif