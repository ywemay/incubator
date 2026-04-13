// #include <SPI.h>
#include "oled.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool Oled::setup() {
  int retry_count = 0;
  bool success = false;
  
  // Try to initialize OLED up to 3 times
  while (retry_count < 3 && !success) {
    Serial.printf("OLED initialization attempt %d...\n", retry_count + 1);
    
    if (display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      success = true;
      display_available = true;
      display.clearDisplay();
      display.setTextSize(2); 
      display.setTextColor(SSD1306_WHITE); 
      display.print("ON");
      display.display();
      delay(1000);
      Serial.println("OLED initialized successfully");
    } else {
      retry_count++;
      if (retry_count < 3) {
        // Short beep for retry
        beep(200, 200);
        delay(1000);
      }
    }
  }
  
  if (!success) {
    // Final failure beep pattern
    beep(400, 600, 4);
    display_available = false;
    Serial.println("OLED initialization failed after 3 attempts. Continuing without display.");
  }
  
  return success;
}

void Oled::display_sensor (int8_t state) {
  if (!display_available) return;
  
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
  } else if (state == -5) {
    display.println("Fail to 0");
  }
  display.display();
}

void Oled::stats(float t, float h, unsigned int turning, unsigned int remained, int8_t state,
                 const String& alt_display) {
  if (!display_available) return;
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0); 

  display.print(t);
  display.print(" ");
  display.print((char)247); // Print the degree symbol
  display.println("C"); 

  if (h == -403) {
    display.print("no RH %");
  } else {
    display.print(h);
    display.print(" %");
  }

  display.setCursor(10, 40);
  display.print("   ");
  display.setCursor(10, 40);

  if (turning) {
    int c = turning % 3;
    if (c == 0) display.print(" ");
    if (c < 2) display.print(" ");
    display.print((char) 16);
  } else {
    // If alt_display is provided, show it instead of remaining time
    if (!alt_display.isEmpty()) {
      display.setTextSize(1); // Smaller text for IP address
      display.print(alt_display);
    } else {
      display.setTextSize(2); // Normal text for time
      unsigned long hours = remained / 3600;
      unsigned long minutes = (remained % 3600) / 60;
      unsigned long seconds = remained % 60;
      display.print(hours);
      display.print("h ");
      display.print(minutes);
      display.print("m ");
      display.print(seconds);
      //display.print("s ");
    }
  }

  // display.print(" ");
  // display.print(state);
  display.display();
}

void Oled::restoreSensor() {
  if (!display_available) return;
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0); 
  display.println("Trying");
  display.println("to restore");
  display.println("sensor.");
  display.display();
}


void Oled::restarting() {
  if (!display_available) return;
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0); 
  display.println("Restart");
  display.display();
}

void Oled::displayIP(const String& ip_address) {
  if (!display_available) return;
  
  display.clearDisplay();
  display.setTextSize(1); // Smaller text for IP address
  display.setCursor(0, 0); 
  display.println("IP Address:");
  display.println("");
  display.setTextSize(2); // Larger text for the IP
  display.println(ip_address);
  display.display();
}