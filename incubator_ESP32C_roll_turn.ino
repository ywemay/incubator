#include "beeper.h"
#include "oled.h"
#include "thermo.h"
#include "turner.h"

Thermo thermo;
Turner turner;
Oled oled;

void setup() {

  setup_beeper();
  oled.setup();
  if(thermo.setup()) {
    oled.display_sensor(0);
    delay(3000);
  }
  else {
    oled.display_sensor(-20);
    while (true) {
      beep(500, 500, 2);
      beep(1500, 500, 2);
      delay(3000);
    }
  }
  turner.setup();  
  beep(200, 300, 2);

}

int8_t state;
uint8_t c = 0;

void loop() {

  if (c == 0) {
    state = thermo.adjust();
  }
  c++;
  if (c >= 4 ) c = 0;

  if (state != 0) {
    oled.display_sensor(state);
  } else {
    unsigned int turning = turner.process();
    oled.stats(
      thermo.temperature(),
      thermo.humidity(),
      turning,
      turner.remained()
    );
  }

  delay(1000);
}
