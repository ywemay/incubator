// #include <SPI.h>
#include "feedback.h"

#ifdef OLED_ON
Oled oled;
#endif
#ifdef LEDS_ON
  Leds leds;
#endif

// IP address for display cycling
String current_ip_address = "";
unsigned long last_display_switch = 0;
bool show_ip_on_display = false;
const unsigned long DISPLAY_SWITCH_INTERVAL = 5000; // Switch every 5 seconds

void Feedback::setup() {
  setup_beeper();
  #ifdef OLED_ON
    if (!oled.setup()) {
      Serial.println("OLED initialization failed. System will continue without display.");
    } else {
      Serial.println("OLED set up successfully.");
    }
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
    // Check if we should switch display mode
    unsigned long current_time = millis();
    if (current_time - last_display_switch > DISPLAY_SWITCH_INTERVAL) {
      show_ip_on_display = !show_ip_on_display;
      last_display_switch = current_time;
    }
    
    // If we have an IP address and it's time to show it, display IP instead of remaining time
    if (show_ip_on_display && !current_ip_address.isEmpty()) {
      oled.stats(t, h, turning, 0, state, current_ip_address);
    } else {
      oled.stats(t, h, turning, remained, state);
    }
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

void Feedback::displayIP(const String& ip_address) {
  #ifdef OLED_ON
    oled.displayIP(ip_address);
  #endif
}

void Feedback::setIPAddress(const String& ip_address) {
  current_ip_address = ip_address;
}

String Feedback::getIPAddress() const {
  return current_ip_address;
}

void Feedback::displayIncubatorInfo(float current_temp, float current_humidity, 
                                   float target_temp, unsigned int turn_interval,
                                   unsigned int incubation_day, unsigned int total_days,
                                   bool wifi_connected, unsigned int turning, 
                                   unsigned int remained, const char* bird_species,
                                   unsigned int candling_day) {
  #ifdef OLED_ON
    oled.displayIncubatorInfo(current_temp, current_humidity, target_temp, 
                             turn_interval, incubation_day, total_days,
                             current_ip_address, wifi_connected, turning, remained,
                             bird_species, candling_day);
  #endif
}