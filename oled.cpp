// #include <SPI.h>
#include "oled.h"
#include <string.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 
#define SCREEN_ADDRESS 0x3C

// Screen cycling interval (seconds)
#define SCREEN_CYCLE_INTERVAL 5000

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);





// Icon definitions (8x8 pixels)
const uint8_t Oled::thermometer_icon[8] = {
  0b00111000, //   ###   
  0b00101000, //   # #   
  0b00111000, //   ###   
  0b00111000, //   ###   
  0b01111100, //  #####  
  0b01111100, //  #####  
  0b01111100, //  #####  
  0b00111000  //   ###   
};

const uint8_t Oled::humidity_icon[8] = {
  0b00111000, //   ###   
  0b01000100, //  #   #  
  0b10000010, // #     # 
  0b10000010, // #     # 
  0b10000010, // #     # 
  0b01000100, //  #   #  
  0b00101000, //   # #   
  0b00010000  //    #    
};

const uint8_t Oled::egg_icon[8] = {
  0b00011000, //    ##   
  0b00111100, //   ####  
  0b01111110, //  ###### 
  0b01111110, //  ###### 
  0b01111110, //  ###### 
  0b01111110, //  ###### 
  0b00111100, //   ####  
  0b00011000  //    ##   
};

const uint8_t Oled::clock_icon[8] = {
  0b00111100, //   ####  
  0b01000010, //  #    # 
  0b10000001, // #      #
  0b10001101, // #   ## #
  0b10001101, // #   ## #
  0b10000001, // #      #
  0b01000010, //  #    # 
  0b00111100  //   ####  
};

const uint8_t Oled::wifi_icon[8] = {
  0b00011000, //    ##   
  0b00100100, //   #  #  
  0b01000010, //  #    # 
  0b00011000, //    ##   
  0b00100100, //   #  #  
  0b00000000, //         
  0b00011000, //    ##   
  0b00000000  //         
};

const uint8_t Oled::target_icon[8] = {
  0b00011000, //    ##   
  0b00111100, //   ####  
  0b01111110, //  ###### 
  0b11111111, // ########
  0b11111111, // ########
  0b01111110, //  ###### 
  0b00111100, //   ####  
  0b00011000  //    ##   
};

const uint8_t Oled::calendar_icon[8] = {
  0b11111111, // ########
  0b10000001, // #      #
  0b10111101, // # #### #
  0b10111101, // # #### #
  0b10000001, // #      #
  0b10111101, // # #### #
  0b10111101, // # #### #
  0b10000001  // #      #
};

// New icons for bird species and candling
const uint8_t Oled::bird_icon[8] = {
  0b00011000, //    ##   
  0b00111100, //   ####  
  0b01111110, //  ###### 
  0b01100110, //  ##  ## 
  0b11111111, // ########
  0b11100111, // ###  ###
  0b01111110, //  ###### 
  0b00111100  //   ####  
};

const uint8_t Oled::candle_icon[8] = {
  0b00011000, //    ##   
  0b00111100, //   ####  
  0b00111100, //   ####  
  0b00111100, //   ####  
  0b01111110, //  ###### 
  0b01111110, //  ###### 
  0b00111100, //   ####  
  0b00011000  //    ##   
};

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

void Oled::displayIncubatorInfo(float current_temp, float current_humidity, 
                               float target_temp, unsigned int turn_interval,
                               unsigned int incubation_day, unsigned int total_days,
                               const String& ip_address, bool wifi_connected,
                               unsigned int turning, unsigned int remained,
                               const String& bird_species, unsigned int candling_day) {
  if (!display_available) return;
  
  // Check if it's time to switch screens
  unsigned long current_time = millis();
  if (current_time - last_screen_change > SCREEN_CYCLE_INTERVAL) {
    current_screen = (current_screen + 1) % 8; // We now have 8 screens (0-7)
    last_screen_change = current_time;
  }
  
  // Clear display
  display.clearDisplay();
  
  // Draw the current screen
  switch (current_screen) {
    case 0:
      drawScreen0(current_temp, current_humidity, turning);
      break;
    case 1:
      drawScreen1(current_temp, target_temp);
      break;
    case 2:
      drawScreen2(turn_interval, remained);
      break;
    case 3:
      drawScreen3(incubation_day, total_days);
      break;
    case 4:
      drawScreen4(ip_address, wifi_connected);
      break;
    case 5:
      drawScreen5(current_temp, current_humidity, target_temp);
      break;
    case 6:
      drawScreen6(bird_species.c_str(), incubation_day, total_days);
      break;
    case 7:
      drawScreen7(candling_day, incubation_day);
      break;
  }
  
  // Display the screen
  display.display();
}

void Oled::drawScreen0(float current_temp, float current_humidity, unsigned int turning) {
  // Screen 0: Current temperature and humidity with icons
  
  // Draw thermometer icon and current temperature
  drawIcon(0, 0, thermometer_icon);
  display.setCursor(12, 0);
  display.setTextSize(2);
  display.print(current_temp, 1);
  display.print((char)247); // Degree symbol
  display.print("C");
  
  // Draw humidity icon and current humidity
  drawIcon(0, 24, humidity_icon);
  display.setCursor(12, 24);
  display.setTextSize(2);
  if (current_humidity == -403) {
    display.print("no RH");
  } else {
    display.print(current_humidity, 0);
    display.print("%");
  }
  
  // Bottom status line
  display.setCursor(0, 48);
  display.setTextSize(1);
  if (turning > 0) {
    display.print("Turning eggs ");
    for (int i = 0; i < 3; i++) {
      if (i < turning % 3) display.print(".");
    }
  } else {
    display.print("Screen 1/8");
  }
}

void Oled::drawScreen1(float current_temp, float target_temp) {
  // Screen 1: Temperature comparison with target
  
  // Header
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Temperature Control");
  
  // Draw current temperature with thermometer icon
  drawIcon(0, 12, thermometer_icon);
  display.setCursor(12, 12);
  display.setTextSize(2);
  display.print("Cur: ");
  display.print(current_temp, 1);
  display.print((char)247);
  
  // Draw target temperature with target icon
  drawIcon(0, 36, target_icon);
  display.setCursor(12, 36);
  display.setTextSize(2);
  display.print("Tar: ");
  display.print(target_temp, 1);
  display.print((char)247);
  
  // Bottom status
  display.setCursor(0, 56);
  display.setTextSize(1);
  display.print("Screen 2/8");
}

void Oled::drawScreen2(unsigned int turn_interval, unsigned int remained) {
  // Screen 2: Egg turning information
  
  // Header
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Egg Turner");
  
  // Draw egg icon
  drawIcon(0, 12, egg_icon);
  display.setCursor(12, 12);
  display.setTextSize(2);
  display.print("Int: ");
  
  // Format interval
  String interval_str = formatInterval(turn_interval);
  display.print(interval_str);
  
  // Draw clock icon and remaining time
  drawIcon(0, 36, clock_icon);
  display.setCursor(12, 36);
  display.setTextSize(2);
  display.print("Rem: ");
  
  if (remained > 0) {
    String time_str = formatTime(remained);
    display.print(time_str);
  } else {
    display.print("N/A");
  }
  
  // Bottom status
  display.setCursor(0, 56);
  display.setTextSize(1);
  display.print("Screen 3/8");
}

void Oled::drawScreen3(unsigned int incubation_day, unsigned int total_days) {
  // Screen 3: Incubation progress
  
  // Header
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Incubation Progress");
  
  // Draw calendar icon
  drawIcon(0, 12, calendar_icon);
  display.setCursor(12, 12);
  display.setTextSize(2);
  
  if (total_days > 0) {
    // Show day progress
    display.print("Day ");
    display.print(incubation_day);
    display.print("/");
    display.print(total_days);
    
    // Draw progress bar
    if (total_days > 0) {
      uint8_t progress = (incubation_day * 100) / total_days;
      drawProgressBar(0, 40, 120, 8, progress, 100);
    }
    
    // Calculate and show remaining days
    if (incubation_day <= total_days) {
      display.setCursor(0, 52);
      display.setTextSize(1);
      display.print("Remaining: ");
      display.print(total_days - incubation_day);
      display.print(" days");
    }
  } else {
    // No active incubation
    display.print("No active");
    display.setCursor(12, 30);
    display.print("incubation");
  }
  
  // Bottom status
  display.setCursor(90, 56);
  display.setTextSize(1);
  display.print("4/8");
}

void Oled::drawScreen4(const String& ip_address, bool wifi_connected) {
  // Screen 4: Network information
  
  // Header
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Network Status");
  
  // Draw WiFi icon
  drawIcon(0, 12, wifi_icon);
  display.setCursor(12, 12);
  display.setTextSize(2);
  
  if (wifi_connected) {
    display.print("Connected");
    
    // Show IP address
    display.setCursor(0, 36);
    display.setTextSize(1);
    display.print("IP: ");
    display.print(ip_address);
    
    // Show hostname
    display.setCursor(0, 48);
    display.print("Host: incubator-esp32");
  } else {
    display.print("No WiFi");
    display.setCursor(12, 30);
    display.print("Connect to");
    display.setCursor(12, 42);
    display.print("setup WiFi");
  }
  
  // Bottom status
  display.setCursor(90, 56);
  display.setTextSize(1);
  display.print("5/8");
}

void Oled::drawScreen5(float current_temp, float current_humidity, float target_temp) {
  // Screen 5: Summary view
  
  // Header
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Incubator Summary");
  
  // Temperature row
  drawIcon(0, 12, thermometer_icon);
  display.setCursor(12, 12);
  display.setTextSize(1);
  display.print("Temp: ");
  display.print(current_temp, 1);
  display.print("/");
  display.print(target_temp, 1);
  display.print((char)247);
  display.print("C");
  
  // Humidity row
  drawIcon(0, 24, humidity_icon);
  display.setCursor(12, 24);
  display.setTextSize(1);
  display.print("Hum:  ");
  if (current_humidity == -403) {
    display.print("no RH");
  } else {
    display.print(current_humidity, 0);
    display.print("%");
  }
  
  // Egg turning row
  drawIcon(0, 36, egg_icon);
  display.setCursor(12, 36);
  display.setTextSize(1);
  display.print("Eggs: Turning OK");
  
  // Status row
  drawIcon(0, 48, wifi_icon);
  display.setCursor(12, 48);
  display.setTextSize(1);
  display.print("WiFi: Connected");
  
  // Bottom status
  display.setCursor(90, 56);
  display.setTextSize(1);
  display.print("6/8");
}

void Oled::drawScreen6(const char* bird_species, unsigned int incubation_day, unsigned int total_days) {
  // Screen 6: Bird species information
  
  // Header
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Bird Species");
  
  // Draw bird icon
  drawIcon(0, 12, bird_icon);
  display.setCursor(12, 12);
  display.setTextSize(2);
  
  if (bird_species && bird_species[0] != '\0') {
    // Show bird species name (truncate if too long)
    char display_name[11]; // 10 chars + null terminator
    strncpy(display_name, bird_species, 10);
    display_name[10] = '\0';
    display.print(display_name);
    
    // Show incubation day info
    display.setCursor(0, 36);
    display.setTextSize(1);
    if (total_days > 0) {
      display.print("Day ");
      display.print(incubation_day);
      display.print(" of ");
      display.print(total_days);
      
      // Calculate percentage
      if (total_days > 0) {
        uint8_t percentage = (incubation_day * 100) / total_days;
        display.setCursor(0, 48);
        display.print("Progress: ");
        display.print(percentage);
        display.print("%");
      }
    } else {
      display.print("No active incubation");
    }
  } else {
    display.print("Not set");
    display.setCursor(12, 30);
    display.print("Select species");
    display.setCursor(12, 42);
    display.print("to start");
  }
  
  // Bottom status
  display.setCursor(90, 56);
  display.setTextSize(1);
  display.print("7/8");
}

void Oled::drawScreen7(unsigned int candling_day, unsigned int incubation_day) {
  // Screen 7: Candling schedule
  
  // Header
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Candling Schedule");
  
  // Draw candle icon
  drawIcon(0, 12, candle_icon);
  display.setCursor(12, 12);
  display.setTextSize(2);
  
  if (candling_day > 0) {
    // Show candling day
    display.print("Day ");
    display.print(candling_day);
    
    // Calculate days until candling
    if (incubation_day < candling_day) {
      unsigned int days_until = candling_day - incubation_day;
      display.setCursor(0, 36);
      display.setTextSize(1);
      display.print("Candle in: ");
      display.print(days_until);
      display.print(" days");
      
      // Show current day
      display.setCursor(0, 48);
      display.print("Today: Day ");
      display.print(incubation_day);
    } else if (incubation_day == candling_day) {
      // TODAY IS CANDLING DAY!
      display.setCursor(0, 36);
      display.setTextSize(1);
      display.print("*** TODAY! ***");
      display.setCursor(0, 48);
      display.print("Candle eggs now");
    } else {
      // Already past candling day
      display.setCursor(0, 36);
      display.setTextSize(1);
      display.print("Candling passed");
      display.setCursor(0, 48);
      display.print("Day ");
      display.print(incubation_day - candling_day);
      display.print(" days ago");
    }
  } else {
    // No candling scheduled for this species
    display.print("No candling");
    display.setCursor(12, 30);
    display.print("scheduled");
    display.setCursor(12, 42);
    display.print("for species");
  }
  
  // Bottom status
  display.setCursor(90, 56);
  display.setTextSize(1);
  display.print("8/8");
}

void Oled::drawIcon(uint8_t x, uint8_t y, const uint8_t* icon) {
  for (uint8_t i = 0; i < 8; i++) {
    for (uint8_t j = 0; j < 8; j++) {
      if (icon[i] & (1 << (7 - j))) {
        display.drawPixel(x + j, y + i, SSD1306_WHITE);
      }
    }
  }
}

void Oled::drawProgressBar(uint8_t x, uint8_t y, uint8_t width, uint8_t height, 
                          uint8_t progress, uint8_t total) {
  // Draw border
  display.drawRect(x, y, width, height, SSD1306_WHITE);
  
  // Calculate fill width
  uint8_t fill_width = 0;
  if (total > 0) {
    fill_width = (progress * (width - 2)) / total;
  }
  
  // Draw filled portion
  if (fill_width > 0) {
    display.fillRect(x + 1, y + 1, fill_width, height - 2, SSD1306_WHITE);
  }
}

String Oled::formatInterval(unsigned int seconds) {
  if (seconds < 60) {
    return String(seconds) + "s";
  } else if (seconds < 3600) {
    return String(seconds / 60) + "m";
  } else {
    unsigned int hours = seconds / 3600;
    unsigned int minutes = (seconds % 3600) / 60;
    return String(hours) + "h" + (minutes < 10 ? "0" : "") + String(minutes) + "m";
  }
}

String Oled::formatTime(unsigned int seconds) {
  unsigned int hours = seconds / 3600;
  unsigned int minutes = (seconds % 3600) / 60;
  unsigned int secs = seconds % 60;
  
  if (hours > 0) {
    return String(hours) + "h" + (minutes < 10 ? "0" : "") + String(minutes) + "m";
  } else if (minutes > 0) {
    return String(minutes) + "m" + (secs < 10 ? "0" : "") + String(secs) + "s";
  } else {
    return String(secs) + "s";
  }
}