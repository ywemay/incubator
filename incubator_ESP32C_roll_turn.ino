#include "beeper.h"
#include "feedback.h"
#include "thermo.h"
#include "turner.h"

Thermo thermo;
Turner turner;
Feedback feedback;

void setup() {

  feedback.setup();
  if(thermo.setup()) {
    feedback.sensor_ok();
  }
  else {
    feedback.sensor_setup_fail();
  }
  turner.setup();  
  feedback.setup_ok();

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
          ESP.restart();
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

  delay(1000);
}
