#ifndef __CONFIG_H__
#define __CONFIG_H__

constexpr unsigned int SENSOR_PIN {5U};
constexpr unsigned int HEATER_PIN {6U};
const unsigned int EGGS_TURNER_PIN = 7;
const unsigned int BEEPER_PIN = 10;

const unsigned int EGGS_TURN_SECONDS = 8;
const unsigned int EGGS_TURNING_INTERVAL = 8 * 60 * 60;

#endif