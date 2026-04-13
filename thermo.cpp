#include "thermo.h"

#ifdef AM2302_SENSOR_PIN
AM2302::AM2302_Sensor am2302{AM2302_SENSOR_PIN};
#endif

bool Thermo::setup() {
  pinMode(HEATER_PIN, OUTPUT);
  #ifdef FAN_PIN
    pinMode(FAN_PIN, OUTPUT);
    fan_off();
  #endif
  stop();
  return sensorBegin();
}

bool Thermo::sensorBegin() {
  #ifdef NTC_SENSOR_PIN
    pinMode(NTC_SENSOR_PIN, INPUT);
  #endif
  #ifdef NTC_SENSOR_POWER_PIN
    pinMode(NTC_SENSOR_POWER_PIN, OUTPUT);
    return true;
  #endif
  #ifdef AM2302_SENSOR_PIN
    return am2302.begin();
  #endif
  
}

void Thermo::fan_on() {
  #ifdef FAN_PIN
    fanOn = true;
    digitalWrite(FAN_PIN, HIGH);
  #endif
}

void Thermo::fan_off() {
  #ifdef FAN_PIN
    fanOn = false;
    digitalWrite(FAN_PIN, LOW);
  #endif
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


void Thermo::ntcPower(uint8_t mode) {
  #ifdef NTC_SENSOR_POWER_PIN
    digitalWrite(NTC_SENSOR_POWER_PIN, mode);
  #endif
}

void Thermo::ntcRead() {
  #ifdef NTC_SENSOR_PIN
    float sample;
    ntcPower(HIGH);
    sample = analogRead(NTC_SENSOR_PIN);
    ntcPower(LOW);
    sample = 1023 * 4 / sample - 1;
    sample = Rref / sample;

    float temperature;
    temperature = sample / nominal_resistance;     // (R/Ro)
    temperature = log(temperature);                  // ln(R/Ro)
    temperature /= beta;                   // 1/B * ln(R/Ro)
    temperature += 1.0 / (nominal_temeprature + 273.15); // + (1/To)
    temperature = 1.0 / temperature;                 // Invert
    temperature -= 273.15;
    t = temperature;
  #endif
}

int8_t Thermo::adjust() {

  int8_t state = 0;

  if (c % 5 == 0) {
    #ifdef AM2302_SENSOR_PIN
      state = am2302.read();
      if (state != AM2302::AM2302_READ_OK) {
        stop();
        return state;
      }
      t = temperature();
      h = humidity();
    #endif
    ntcRead();
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
    // ESP.restart();
  }

  if (runsCount <=2) runsCount++;

  if (t <= targetTemp - 0.3) {
    heat();
    fan_on();
  } else if (t <= targetTemp) {
    intermitentHeat();
    fan_on();
  } else {
    stop();
    fan_off();
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
  #ifdef AM2302_SENSOR_PIN
    return am2302.get_Temperature();
  #endif
  #ifdef NTC_SENSOR_PIN
    return t;
  #endif
  return -403;
}

float Thermo::humidity(){
  #ifdef AM2302_SENSOR_PIN
    return am2302.get_Humidity();
  #endif
  return -403;
}