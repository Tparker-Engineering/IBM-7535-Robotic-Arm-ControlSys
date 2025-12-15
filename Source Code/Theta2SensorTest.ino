#include "OptaBlue.h"

/*Code written and tested by Rewired and Rearmed. For future senior design teams, Theta2 Arm must be position slightly off the the home sensor
arm will drift into the home postion during initialization and follow the test. Test should go to the ccw overrun sensor then to the cw overun sensor and back to home.*/

enum RobotState { IDLE, TO_POS, WAIT, TO_NEG, TO_HOME, DONE };
RobotState state = IDLE;

unsigned long waitStartTime = 0;
bool waitStarted = false;

AnalogExpansion aexp;

void setup() {
  Serial.begin(115200);
  delay(1000);

  OptaController.begin();

  pinMode(A3, INPUT_PULLUP);  // I4 - θ₂ home
  pinMode(A4, INPUT_PULLUP);  // I6 - θ₂ overrun−
  pinMode(A5, INPUT_PULLUP);  // I5 - θ₂ overrun+

  while (OptaController.getExpansionNum() == 0) {
    Serial.println("Waiting for analog expansion...");
    OptaController.update();
    delay(500);
  }

  aexp = OptaController.getExpansion(0);

  aexp.beginChannelAsDac(5, OA_VOLTAGE_DAC, true, false, OA_SLEW_RATE_0);
  aexp.beginChannelAsDac(6, OA_VOLTAGE_DAC, true, false, OA_SLEW_RATE_0);
  aexp.pinVoltage(5, 0.0f);
  aexp.pinVoltage(6, 0.0f);
  OptaController.update();

  Serial.println("Setup complete. Starting in IDLE.");
}

void loop() {
  bool theta2_home = (digitalRead(A3) == LOW);        // I4
  bool theta2_overrunPos = (digitalRead(A5) == LOW);  // I6
  bool theta2_overrunNeg = (digitalRead(A4) == LOW);  // I5

  switch (state) {
    case IDLE:
      aexp.pinVoltage(5, 0.0f);
      aexp.pinVoltage(6, 0.0f);
      OptaController.update();

      if (theta2_home) {
        Serial.println("θ₂ home detected. Moving to overrunPos.");
        aexp.pinVoltage(5, 0.0f);
        aexp.pinVoltage(6, 4.5f);  // CCW
        OptaController.update();
        state = TO_POS;
      }
      break;

    case TO_POS:
      if (theta2_overrunPos) {
        Serial.println("Reached overrunPos. Stopping and waiting.");
        aexp.pinVoltage(5, 0.0f);
        aexp.pinVoltage(6, 0.0f);
        OptaController.update();
        waitStartTime = millis();
        waitStarted = true;
        state = WAIT;
      }
      break;

    case WAIT:
      if (waitStarted && millis() - waitStartTime >= 1000) {
        Serial.println("Wait done. Re-initializing DACs and moving to overrunNeg.");

        aexp.beginChannelAsDac(5, OA_VOLTAGE_DAC, true, false, OA_SLEW_RATE_0);
        aexp.beginChannelAsDac(6, OA_VOLTAGE_DAC, true, false, OA_SLEW_RATE_0);
        delay(10);

        aexp.pinVoltage(5, 5.2f);  // CW
        aexp.pinVoltage(6, 0.0f);
        OptaController.update();

        state = TO_NEG;
      }
      break;

    case TO_NEG:
      if (theta2_overrunNeg) {
        Serial.println("Reached overrunNeg. Returning to home.");

        aexp.beginChannelAsDac(5, OA_VOLTAGE_DAC, true, false, OA_SLEW_RATE_0);
        aexp.beginChannelAsDac(6, OA_VOLTAGE_DAC, true, false, OA_SLEW_RATE_0);
        delay(10);

        aexp.pinVoltage(5, 0.0f);
        aexp.pinVoltage(6, 4.5f);  // CCW return
        OptaController.update();

        state = TO_HOME;
      }
      break;

    case TO_HOME:
      if (theta2_home) {
        Serial.println("Returned to home. Final stop.");
        aexp.pinVoltage(5, 0.0f);
        aexp.pinVoltage(6, 0.0f);
        OptaController.update();
        state = DONE;
      }
      break;

    case DONE:
      // Remain stopped
      break;
  }

  delay(50);
}
