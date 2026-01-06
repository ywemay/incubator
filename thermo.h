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
    bool fanOn = false;
    bool setup();
    bool sensorBegin();
    void heat();
    void fan_on();
    void fan_off();
    void intermitentHeat();
    void stop();
    int8_t adjust();
    float temperature();
    float humidity();
  private:
    void ntcRead();
    void ntcPower(uint8_t mode);
};

#endif