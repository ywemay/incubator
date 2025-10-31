#ifndef __THERMO__h__
#define __THERMO__h__

#include "Arduino.h"
#include <AM2302-Sensor.h>
#include "config.h"
#include "oled.h"
#include "beeper.h"

class Thermo {

  public:
    Thermo() {};
    bool setup();
    bool sensorBegin();
    void heat();
    void intermitentHeat();
    void stop();
    int8_t adjust();
    float temperature();
    float humidity();
};

#endif