#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "beeper.h"

class Oled {
  private:
    bool display_available;
    unsigned long last_screen_change;
    uint8_t current_screen;
    
    // Icons (8x8 pixels)
    static const uint8_t thermometer_icon[8];
    static const uint8_t humidity_icon[8];
    static const uint8_t egg_icon[8];
    static const uint8_t clock_icon[8];
    static const uint8_t wifi_icon[8];
    static const uint8_t target_icon[8];
    static const uint8_t calendar_icon[8];
    
  public:
    Oled() : display_available(false), last_screen_change(0), current_screen(0) {};
    bool setup();
    void display_sensor(int8_t state);
    void stats(float t, float h, unsigned int turning, unsigned int remained, int8_t state, 
               const String& alt_display = "");
    void restoreSensor();
    void restarting();
    void displayIP(const String& ip_address);
    
    // New cycling display system
    void displayIncubatorInfo(float current_temp, float current_humidity, 
                             float target_temp, unsigned int turn_interval,
                             unsigned int incubation_day, unsigned int total_days,
                             const String& ip_address, bool wifi_connected,
                             unsigned int turning = 0, unsigned int remained = 0,
                             const String& bird_species = "", 
                             unsigned int candling_day = 0);
    
    // Check if display is available
    bool isAvailable() const { return display_available; }
    
  private:
    // Screen drawing methods
    void drawScreen0(float current_temp, float current_humidity, unsigned int turning);
    void drawScreen1(float current_temp, float target_temp);
    void drawScreen2(unsigned int turn_interval, unsigned int remained);
    void drawScreen3(unsigned int incubation_day, unsigned int total_days);
    void drawScreen4(const String& ip_address, bool wifi_connected);
    void drawScreen5(float current_temp, float current_humidity, float target_temp);
    
    // Helper methods
    void drawIcon(uint8_t x, uint8_t y, const uint8_t* icon);
    void drawProgressBar(uint8_t x, uint8_t y, uint8_t width, uint8_t height, 
                        uint8_t progress, uint8_t total);
    String formatInterval(unsigned int seconds);
    String formatTime(unsigned int seconds);
};

#endif