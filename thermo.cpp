#include <thermo.h>

AM2302::AM2302_Sensor am2302{SENSOR_PIN};

const float minTemp = 37.7;
const float maxTemp = 38.1;

bool Thermo::setup() {
  pinMode(HEATER_PIN, OUTPUT);
  stop();
  return am2302.begin();

}

void Thermo::heat() {
  digitalWrite(HEATER_PIN, HIGH);
}

void Thermo::stop() {
  digitalWrite(HEATER_PIN, LOW);
}

uint8_t runsCount = 0;

int8_t Thermo::adjust() {

  int8_t state = am2302.read();

  if (state != AM2302::AM2302_READ_OK) {
    stop();
    return state;
  }

  float t = temperature();
  float h = humidity();

  if (t == 0.00 && h == t) {
    stop();
    return -5;
  }

  if (t > maxTemp + 1) {
    digitalWrite(HEATER_PIN, 0);
    beep(1000, 200, 3);
    // wait for cooling
    if (runsCount < 2) delay(5000);
    ESP.restart();
  }

  if (runsCount <=2) runsCount++;

  if (t <= (minTemp + maxTemp) / 2) {
    heat();
  } else {
    stop();
  }

  return state;
}

float Thermo::temperature() {
  return am2302.get_Temperature();
}

float Thermo::humidity(){
  return am2302.get_Humidity();
}