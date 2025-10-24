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
  Serial.println("Heater pin low.");
  // digitalWrite(HEATER_PIN, LOW);
  analogWrite(HEATER_PIN, 0);
}

int8_t Thermo::adjust() {

  int8_t state = am2302.read();

  if (state != AM2302::AM2302_READ_OK) {
    stop();
    return state;
  }

  float t = am2302.get_Temperature();

  Serial.print("Adjusting: ");
  Serial.print(t);
  Serial.print(" -> ");
  
  float minTempReact = minTemp;

  if (t > maxTemp + 1) beep(1000, 200, 3);

  if (t >= maxTemp) {
    stop();
    Serial.println("stop");
  } else if (t < minTempReact ) {
    heat();
    Serial.println("too low, heat");
  } else if (t < maxTemp - 0.1 && t >= minTempReact) {
    int v = map(t * 10 , maxTemp * 10, minTempReact * 10, 100, 255);
    heat(v);
    Serial.print("moderate, ");
    Serial.println(v);
  } else {
    stop();
    Serial.println("watever, stop");
  }

  return state;
}

float Thermo::temperature() {
  return am2302.get_Temperature();
}

float Thermo::humidity(){
  return am2302.get_Humidity();
}