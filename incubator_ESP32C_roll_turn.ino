#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <AM2302-Sensor.h>

constexpr unsigned int SENSOR_PIN {5U};
AM2302::AM2302_Sensor am2302{SENSOR_PIN};

const float minTemp = 37.5;
const float maxTemp = 38.1;

const unsigned int HEATER_PIN = 6;
const unsigned int EGGS_TURNER_PIN = 7;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 
#define SCREEN_ADDRESS 0x3C 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int eggsTurnCounter = 10;

void setup_display() {
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    for(;;);
  }

  display.clearDisplay();
  display.setTextSize(2); 
  display.setTextColor(SSD1306_WHITE); 
  display.print("Incubator");
  display.display();
  delay(3000);

}

void setup_sensor() {

  display.clearDisplay();
  display.println("Temp sensor");
  if (am2302.begin()) {
    display.println("OK");
    display.display();
    delay(3000);
  }
  else {
    display.println("Error");
    display.display();
    while (true) {
      delay(10000);
    }
  }
  
}

void setup_heater() {
  pinMode(HEATER_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, LOW);
}

void setup_eggs_turner() {
  pinMode(EGGS_TURNER_PIN, OUTPUT);
  digitalWrite(EGGS_TURNER_PIN, LOW);
}

void setup() {

  setup_display();
  setup_sensor();
  setup_heater();
  setup_eggs_turner();  
}

void loop() {

  int temperature = 0;
  int humidity = 0;

  float t = am2302.get_Temperature();
  float h = am2302.get_Humidity();

  /* 
  Simple temperature logic
  if (t < minTemp ) {
    digitalWrite(HEATER_PIN, HIGH);
  } else if (t > maxTemp ) {
    digitalWrite(HEATER_PIN, LOW);
  }
  */
  
  if (t < minTemp - 6) {
    digitalWrite(HEATER_PIN, HIGH);
  } else if (t < maxTemp) {
    int v = map(t, minTemp, maxTemp, 0, 255);
    analogWrite(HEATER_PIN, 255 - v);
  } else if (t > maxTemp) {
    digitalWrite(HEATER_PIN, LOW);
  }

  auto status = am2302.read();

  
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0); 
  
  display.print(t);
  display.print(" ");
  display.print((char)247); // Print the degree symbol
  display.println("C"); 

  display.print(h);
  display.print(" %");

  eggsTurnCounter--;

  display.setCursor(10, 40);
  display.print("   ");
  display.setCursor(10, 40);

  if (eggsTurnCounter <= 0) {
    digitalWrite(EGGS_TURNER_PIN, LOW);
    eggsTurnCounter = 8 * 60 * 6;
    display.print(eggsTurnCounter);  
  } else if (eggsTurnCounter <= 8) {
    digitalWrite(EGGS_TURNER_PIN, HIGH);
    int c = eggsTurnCounter % 3;
    if (c == 0) display.print(" ");
    if (c < 2) display.print(" ");
    display.print((char) 16);
  } else {
    display.print("   ");
    display.print(eggsTurnCounter);
  }

  display.setCursor(50, 40);
  display.display();

  delay(1000);
}
