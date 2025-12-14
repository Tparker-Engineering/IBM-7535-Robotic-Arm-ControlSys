#include "OptaBlue.h"

/*Code written and tested by Rewired and Rearmed. For future senior design teams, Theta1 Arm must be position slightly before the the home roller cam
arm will drift into the home postion during initialization and follow the test. Test should go to the far overrun sensor and back to home.*/

enum RobotState { IDLE, TO_POS, TO_NEG, TO_HOME, DONE };
RobotState state = IDLE;

AnalogExpansion aexp;

void setup() {
  Serial.begin(115200);
  delay(1000);

  OptaController.begin();

  pinMode(A0, INPUT_PULLUP);  // I1 - θ₁ home
  pinMode(A1, INPUT_PULLUP);  // I2 - θ₁ overrun (shared)

  while (OptaController.getExpansionNum() == 0) {
    Serial.println("Waiting for analog expansion...");
    OptaController.update();
    delay(500);
  }

  aexp = OptaController.getExpansion(0);

  aexp.beginChannelAsDac(2, OA_VOLTAGE_DAC, true, false, OA_SLEW_RATE_0);
  aexp.beginChannelAsDac(3, OA_VOLTAGE_DAC, true, false, OA_SLEW_RATE_0);
  aexp.pinVoltage(2, 0.0f);
  aexp.pinVoltage(3, 0.0f);
  OptaController.update();

  Serial.println("Setup complete. Starting in IDLE.");
}

void loop() {
  // Software pull-up hack: θ₁ home 
  pinMode(A0, OUTPUT);
  digitalWrite(A0, HIGH);       // Pull high
  delayMicroseconds(10);
  pinMode(A0, INPUT);           // Float and wait for sensor
  delayMicroseconds(5);
  bool theta1_home = (digitalRead(A0) == LOW);  // Active low if sensor pulls down

  // Software pull-up hack: θ₁ overrun 
  pinMode(A1, OUTPUT);
  digitalWrite(A1, HIGH);
  delayMicroseconds(10);
  pinMode(A1, INPUT);
  delayMicroseconds(5);
  bool theta1_overrun = (digitalRead(A1) == LOW);

  Serial.print("theta1_home: ");
  Serial.print(theta1_home ? "ACTIVE " : "INACTIVE ");
  Serial.print(" | theta1_overrun: ");
  Serial.println(theta1_overrun ? "ACTIVE" : "INACTIVE");

  switch (state) {
    case IDLE:
      aexp.pinVoltage(2, 0.0f);
      aexp.pinVoltage(3, 0.0f);
      OptaController.update();

      if (theta1_home) {
        Serial.println("θ₁ home detected. Moving to overrun (CCW).");
        aexp.pinVoltage(2, 0.0f);
        aexp.pinVoltage(3, 4.5f);  // CCW
        OptaController.update();
        state = TO_POS;
      }
      break;

    case TO_POS:
      if (theta1_overrun) {
        Serial.println("Reached overrun. Reversing CW to home.");

        aexp.beginChannelAsDac(2, OA_VOLTAGE_DAC, true, false, OA_SLEW_RATE_0);
        aexp.beginChannelAsDac(3, OA_VOLTAGE_DAC, true, false, OA_SLEW_RATE_0);
        delay(10);

        aexp.pinVoltage(2, 5.2f);  // CW
        aexp.pinVoltage(3, 0.0f);
        OptaController.update();

        state = TO_HOME;
      }
      break;

    case TO_HOME:
      if (theta1_home) {
        Serial.println("Returned to home. Final stop.");

        aexp.beginChannelAsDac(2, OA_VOLTAGE_DAC, true, false, OA_SLEW_RATE_0);
        aexp.beginChannelAsDac(3, OA_VOLTAGE_DAC, true, false, OA_SLEW_RATE_0);
        delay(10);

        aexp.pinVoltage(2, 0.0f);
        aexp.pinVoltage(3, 0.0f);
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


