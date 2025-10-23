// #include <SPI.h>
#include "oled.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 
#define SCREEN_ADDRESS 0x3C 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void Oled::setup() {

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    
    beep(400, 600, 4);
    delay(2000);
    for(;;);
  }

  display.clearDisplay();
  display.setTextSize(2); 
  display.setTextColor(SSD1306_WHITE); 
  display.print("ON");
  display.display();
  delay(1000);

}

void Oled::display_sensor (int8_t state) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(2, 3); 
  display.println("Sensor");
  if (state == 0) {
    display.println("OK");
  }
  else {
    display.println("Error");
  }

  if (state == -1) {
    display.println("Checksum");
  } else if (state == -2) {
    display.println("Timeout");
  } else if (state == -3) {
    display.println("Read freq.");
  }
  display.display();
}

void Oled::stats(float t, float h, unsigned int turning, unsigned int remained) {
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0); 

  display.print(t);
  display.print(" ");
  display.print((char)247); // Print the degree symbol
  display.println("C"); 

  display.print(h);
  display.print(" %");

  display.setCursor(10, 40);
  display.print("   ");
  display.setCursor(10, 40);

  if (turning) {
    int c = turning % 3;
    if (c == 0) display.print(" ");
    if (c < 2) display.print(" ");
    display.print((char) 16);
  } else {
    display.print(remained);
  }

  display.display();
}