#include "turner.h"
#include "config.h"

unsigned int eggsTurnCounter = EGGS_TURN_SECONDS;
#ifdef EGGS_TURNER_SERVO_PIN
Servo eggsTurnServo;
uint8_t currentServoDegrees = 0;
int8_t servoDirection = EGGS_TURN_SERVER_STEPS;
#endif

void Turner::setup() {
  
  #ifdef EGGS_TURNER_PIN
    pinMode(EGGS_TURNER_PIN, OUTPUT);
    digitalWrite(EGGS_TURNER_PIN, LOW);
  #endif

  #ifdef EGGS_TURNER_SERVO_PIN
    eggsTurnServo.attach(EGGS_TURNER_SERVO_PIN);
    delay(15);
    eggsTurnServo.write(0);
  #endif
}

unsigned int Turner::process() {
  eggsTurnCounter--;
  
  if (eggsTurnCounter <= 0) {
    stop();
    eggsTurnCounter = EGGS_TURNING_INTERVAL;
  } else if (eggsTurnCounter <= EGGS_TURN_SECONDS) {
    turn();
    return EGGS_TURN_SECONDS + 1 - eggsTurnCounter;
  }
  return 0; 
}

void Turner::turn() {
  #ifdef EGGS_TURNER_PIN
    digitalWrite(EGGS_TURNER_PIN, HIGH);
  #endif

  #ifdef EGGS_TURNER_SERVO_PIN
    currentServoDegrees += servoDirection;
    if (currentServoDegrees > 180 || currentServoDegrees < 0) {
      servoDirection = -servoDirection;
      eggsTurnCounter = 0; // stop turnin process - not to go forth and back
    }
    eggsTurnServo.write(currentServoDegrees);
  #endif
}

void Turner::stop() {
  #ifdef EGGS_TURNER_PIN
    digitalWrite(EGGS_TURNER_PIN, LOW);
  #endif
}

unsigned int Turner::remained() { return eggsTurnCounter - EGGS_TURN_SECONDS + 1; }