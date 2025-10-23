#include "turner.h"

void Turner::setup() {
  pinMode(EGGS_TURNER_PIN, OUTPUT);
  digitalWrite(EGGS_TURNER_PIN, LOW);
}

unsigned int eggsTurnCounter = 10;

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

unsigned int Turner::remained() { return eggsTurnCounter - EGGS_TURN_SECONDS + 1; }