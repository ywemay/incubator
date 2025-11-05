#ifndef __CONFIG_H__
#define __CONFIG_H__

// use or not Oled screen
#define OLED_ON
// LED based feedback
// #define LEDS_ON

#ifdef LEDS_ON
#define LED_COLD A0
#define LED_OK A1
#define LED_HOT A2
#endif

// target temperature, celsius
const float targetTemp = 38.0;

constexpr unsigned int SENSOR_PIN {5U};
constexpr unsigned int HEATER_PIN {6U};

#define EGGS_TURNER_PIN 7
// #define EGGS_TURNER_SERVO_PIN 21
const unsigned int BEEPER_PIN = 10;

#ifdef EGGS_TURNER_PIN
const unsigned int EGGS_TURN_SECONDS = 2;
#endif
#ifdef EGGS_TURNER_SERVO_PIN
const unsigned int EGGS_TURN_SERVER_STEPS = 5;
const unsigned int EGGS_TURN_SECONDS = (unsigned int) (180 / EGGS_TURN_SERVER_STEPS);
#endif

const unsigned int EGGS_TURNING_INTERVAL = 8 * 60 * 60;

#endif