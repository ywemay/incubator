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

// comment/uncomment to enable/disable the usage of NTC type sersor 
// #define NTC_SENSOR_PIN A3
//#define NTC_SENSOR_POWER_PIN A4

// NTC related
#ifdef NTC_SENSOR_PIN
#define nominal_resistance 10900       //Nominal resistance at 25⁰C
#define nominal_temeprature 25   // temperature for nominal 
#define beta 4000  // The beta coefficient or the B value of the thermistor (usually 3000-4000) check the datasheet for the accurate value.
#define Rref 9890   //Value of  resistor used for the voltage divider
#endif

#define FAN_PIN 20
#define LIGHT_PIN 21
// comment/uncomment to enable/disable the usage of AM2302 type thermo/humidity sensor
#define AM2302_SENSOR_PIN 5
constexpr unsigned int HEATER_PIN {6U};

#define EGGS_TURNER_PIN 10
// #define EGGS_TURNER_SERVO_PIN 21
#define BEEPER_PIN 7

#ifdef EGGS_TURNER_PIN
const unsigned int EGGS_TURN_SECONDS = 2;
#endif
#ifdef EGGS_TURNER_SERVO_PIN
const unsigned int EGGS_TURN_SERVER_STEPS = 5;
const unsigned int EGGS_TURN_SECONDS = (unsigned int) (180 / EGGS_TURN_SERVER_STEPS);
#endif

const unsigned int EGGS_TURNING_INTERVAL = 8 * 60 * 60;

#endif