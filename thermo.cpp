#include <thermo.h>

AM2302::AM2302_Sensor am2302{SENSOR_PIN};

bool Thermo::setup() {
  pinMode(HEATER_PIN, OUTPUT);
  stop();
  return sensorBegin();
}

bool Thermo::sensorBegin() {
  return am2302.begin();
}

void Thermo::heat() {
  digitalWrite(HEATER_PIN, HIGH);
}

void Thermo::stop() {
  digitalWrite(HEATER_PIN, LOW);
}

uint8_t runsCount = 0;
uint8_t c = 0;
float t = 0;
float h = 6;

int8_t Thermo::adjust() {

  int8_t state;

  if (c % 5 == 0) {
    state = am2302.read();
    if (state != AM2302::AM2302_READ_OK) {
      stop();
      return state;
    }
    t = temperature();
    h = humidity();
  }
  c++;

  if (t == 0.00 && h == t) {
    stop();
    return -5;
  }

  if (t > targetTemp + 1) {
    digitalWrite(HEATER_PIN, 0);
    beep(1000, 200, 3);
    // wait for cooling
    if (runsCount < 2) delay(5000);
    ESP.restart();
  }

  if (runsCount <=2) runsCount++;

  if (t <= targetTemp - 0.3) {
    heat();
  } else if (t <= targetTemp) {
    intermitentHeat();
  } else {
    stop();
  }

  return state;
}

uint8_t intermitentHeatCounter = 0;
uint8_t intermitentHeatFrequency = 3;

void Thermo::intermitentHeat() {
  intermitentHeatCounter++;
  if (intermitentHeatCounter % intermitentHeatFrequency) heat();
  else stop();
}

float Thermo::temperature() {
  return am2302.get_Temperature();
}

float Thermo::humidity(){
  return am2302.get_Humidity();
}