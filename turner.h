#ifndef __TURNER_H__
#define __TURNER_H__

#include <Arduino.h>
#include "config.h"

class Turner {

  public:
    // Turner() {};
    void setup();
    unsigned int process();
    void turn() { digitalWrite(EGGS_TURNER_PIN, HIGH); };
    void stop() { digitalWrite(EGGS_TURNER_PIN, LOW); };
    unsigned int remained();
};

#endif