#include "beeper.h"
#include "feedback.h"
#include "thermo.h"
#include "turner.h"

// WiFi and Web Server features (ESP32 only)
#ifdef ESP32
#include "wifi_manager.h"
#include "web_server.h"
#include "config_storage.h"
#include "incubation_tracker.h"
#endif

// Global objects
Thermo thermo;
Turner turner;
Feedback feedback;

#ifdef ESP32
// WiFi and Web Server objects
WiFiManager wifiManager;
WebServerManager webServer;
ConfigStorage configStorage;
IncubationTracker incubationTracker;

// Global configuration variables (defined here for ESP32)
float targetTemp = 38.0;
unsigned int EGGS_TURNING_INTERVAL = 8 * 60 * 60;

#ifdef EGGS_TURNER_PIN
unsigned int EGGS_TURN_SECONDS = 2;
#endif

#ifdef EGGS_TURNER_SERVO_PIN
unsigned int EGGS_TURN_SERVER_STEPS = 5;
unsigned int EGGS_TURN_SECONDS = (unsigned int)(180 / EGGS_TURN_SERVER_STEPS);
#endif
#endif

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ESP32 Incubator Controller ===");
  
  // Load configuration (ESP32 only)
  #ifdef ESP32
  if (configStorage.begin()) {
    targetTemp = configStorage.loadTargetTemperature(38.0);
    EGGS_TURNING_INTERVAL = configStorage.loadTurnInterval(8 * 60 * 60);
    #ifdef EGGS_TURNER_PIN
    EGGS_TURN_SECONDS = configStorage.loadTurnDuration(2);
    #endif
    configStorage.end();
  }
  Serial.printf("Configuration loaded: Temp=%.1f°C, Interval=%u sec\n", 
                targetTemp, EGGS_TURNING_INTERVAL);
  #endif
  
  // Initialize hardware components
  feedback.setup();
  if(thermo.setup()) {
    feedback.sensor_ok();
  } else {
    feedback.sensor_setup_fail();
  }
  turner.setup();  
  feedback.setup_ok();
  
  // Initialize WiFi and Web Server (ESP32 only)
  #ifdef ESP32
  wifiManager.begin();
  webServer.begin();
  incubationTracker.begin();
  
  // Display IP address on OLED if WiFi is connected
  if (wifiManager.isConnected()) {
    String ip_address = wifiManager.getIPAddress();
    Serial.printf("WiFi connected. IP: %s\n", ip_address.c_str());
    feedback.setIPAddress(ip_address); // Store for cycling display
    feedback.displayIP(ip_address);
    delay(3000); // Show IP for 3 seconds
  }
  #endif
  
  Serial.println("System initialization complete");
}

int8_t state;
uint8_t errorsCount = 0;
uint8_t restoreTries = 0;

void handleErrors() {
  if (state != 0) {
    errorsCount++;
    if (errorsCount > 2) {
      if (state == -5) {
        if (restoreTries > 2) {
          feedback.restarting();
          // ESP.restart();
          // restart();
        } 
        feedback.restoreSensor();
        thermo.sensorBegin();
        delay(1000);
        restoreTries++;
      }
      feedback.error();
    }
  } else {
    errorsCount = 0;
    restoreTries = 0;
  }
}

void loop() {
  // Temperature control loop
  state = thermo.adjust();
  handleErrors();

  if (state != 0) {
    feedback.display_sensor(state);
  } else {
    unsigned int turning = turner.process();
    feedback.stats(
      thermo.temperature(),
      thermo.humidity(),
      turning,
      turner.remained(),
      state
    );
  }
  
  // Handle WiFi and Web Server (ESP32 only)
  #ifdef ESP32
  wifiManager.loop();
  webServer.loop();
  
  // Check for WiFi connection status changes and display IP
  static bool last_wifi_status = false;
  bool current_wifi_status = wifiManager.isConnected();
  if (current_wifi_status && !last_wifi_status) {
    // WiFi just connected
    String ip_address = wifiManager.getIPAddress();
    Serial.printf("WiFi connected. IP: %s\n", ip_address.c_str());
    feedback.setIPAddress(ip_address); // Store for cycling display
    feedback.displayIP(ip_address);
    delay(2000); // Show IP for 2 seconds
  } else if (!current_wifi_status && last_wifi_status) {
    // WiFi disconnected
    feedback.setIPAddress(""); // Clear IP address
  }
  last_wifi_status = current_wifi_status;
  
  // Check incubation status (once per minute)
  static unsigned long last_incubation_check = 0;
  if (millis() - last_incubation_check > 60000) { // Every minute
    last_incubation_check = millis();
    
    if (incubationTracker.isSessionActive()) {
      // Check for important days
      if (incubationTracker.isCandlingDay()) {
        Serial.println("*** TODAY IS CANDLING DAY! ***");
        // You could add a special beep pattern here
      }
      
      if (incubationTracker.isLockdownDay()) {
        Serial.println("*** LOCKDOWN DAY - STOP TURNING EGGS ***");
        // You could disable egg turning here
      }
      
      if (incubationTracker.isHatchingDay()) {
        Serial.println("*** HATCHING DAY! ***");
        // You could add a special notification here
      }
    }
  }
  #endif
  
  delay(1000);
}
