#include "OptaBlue.h"

/*Test will spin stepper clockwise and stop. There is a metal belt clamp on the stepper this should be position so that it does not hit gears! OA_PWM_CH_1 would spin the stepper in the other direction*/

void setup() {
  OptaController.begin();

  while (OptaController.getExpansionNum() == 0) {
    OptaController.update();
    delay(500);
  }

  Opta::AnalogExpansion aexp = (Opta::AnalogExpansion&)OptaController.getExpansion(0);

  // Spin for 0.25 seconds
  aexp.setPwm(OA_PWM_CH_0, 1600, 25);  // 1600 Hz, 25% duty
  delay(400);

  // Stop motor
  aexp.setPwm(OA_PWM_CH_0, 1600, 0);  // 0% duty (off)
}

void loop() {
  // Do nothing — motor has stopped
  while (1)
    ;  // Prevent further execution
}
