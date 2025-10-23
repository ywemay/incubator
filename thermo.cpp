#include <thermo.h>

AM2302::AM2302_Sensor am2302{SENSOR_PIN};

const float minTemp = 37.5;
const float maxTemp = 38.1;

bool Thermo::setup() {

  pinMode(HEATER_PIN, OUTPUT);
  stop();
  return am2302.begin();

}

void Thermo::heat(uint8_t power) {
  analogWrite(HEATER_PIN, power);
}

void Thermo::stop() {
  digitalWrite(HEATER_PIN, LOW);
}

int8_t Thermo::adjust() {

  int8_t state = am2302.read();

  if (state != AM2302::AM2302_READ_OK) {
    stop();
    beep(1000, 1000);
    return state;
  }

  float t = am2302.get_Temperature();

  if (t < minTemp - 6) {
    heat();
  } else if (t < maxTemp) {
    int v = map(t, minTemp, maxTemp, 0, 255);
    heat(255 - v);
  } else if (t > maxTemp) {
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