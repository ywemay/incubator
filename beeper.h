#ifndef __BEEPER_H__
#define __BEEPER_H__

#include "Arduino.h"
#include "config.h"

void setup_beeper();
void beep(unsigned long sec = 0, unsigned long pause = 0, unsigned int repeat = 1);

#endif