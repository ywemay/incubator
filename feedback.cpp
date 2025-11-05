// #include <SPI.h>
#include "feedback.h"

#ifdef OLED_ON
Oled oled;
#endif
#ifdef LEDS_ON
  Leds leds;
#endif

void Feedback::setup() {
  setup_beeper();
  #ifdef OLED_ON
    oled.setup();
  #endif
  #ifdef LEDS_ON
    leds.setup();
  #endif
}

void Feedback::sensor_ok() {
  #ifdef OLED_ON
  display_sensor(0);
  delay(500);
  #endif
}

void Feedback::setup_ok() {
  beep(200, 300, 2);
}

void Feedback::error() {
  beep(1000, 1000);
}

void Feedback::sensor_setup_fail() {
  display_sensor(-20);
  #ifdef LEDS_OK
  leds.all(OFF);
  #endif
  
  while (true) {
    #ifdef LEDS_ON
      leds.hot_beep(500, 500, 2);
      leds.hot_beep(1500, 500, 2);
      delay(3000);
    #else
      beep(500, 500, 2);
      beep(1500, 500, 2);
      delay(3000);
    #endif
  }
}

void Feedback::display_sensor (int8_t state) {
  #ifdef OLED_ON
    oled.display_sensor(state);
  #endif
}

void Feedback::stats(float t, float h, unsigned int turning, unsigned int remained, int8_t state) {
  #ifdef OLED_ON
    oled.stats(t, h, turning, remained, state);
  #endif

  #ifdef LEDS_ON
    float minTemp = targetTemp - 0.5;
    float maxTemp = targetTemp + 0.5;
    leds.hot(t > maxTemp ? ON : OFF);
    leds.ok(t <= maxTemp && t >= minTemp ? ON : OFF);
    leds.cold(t < minTemp ? ON : OFF);
  #endif
}

void Feedback::restoreSensor() {
  #ifdef OLED_ON
    oled.restoreSensor();
  #endif
}


void Feedback::restarting() {
#ifdef OLED_ON
    oled.restarting();
    delay(500);
  #endif
}