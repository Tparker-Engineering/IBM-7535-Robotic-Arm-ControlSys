#include "OptaBlue.h"

bool testDone = false;

void setup() {
  Serial.begin(115200);
  delay(1000);

  OptaController.begin();

  // Wait until the analog expansion is detected
  while (OptaController.getExpansionNum() == 0) {
    Serial.println("Waiting for expansion...");
    OptaController.update();
    delay(500);
  }

  // Get the expansion object
  AnalogExpansion exp = OptaController.getExpansion(0);

  if (exp) {
    // Correct usage: non-static method, 5 arguments
    exp.beginChannelAsDac(5, OA_VOLTAGE_DAC, true, false, OA_SLEW_RATE_0);  // I5
    exp.beginChannelAsDac(6, OA_VOLTAGE_DAC, true, false, OA_SLEW_RATE_0);  // I6
    delay(1000);  // Let DAC stabilize
  }
}

void loop() {
  AnalogExpansion exp = OptaController.getExpansion(0);

  if (!testDone && exp) {
    Serial.println("Motor 2 -2.5V");
    exp.pinVoltage(5, 4.6f, true);
    exp.pinVoltage(6, 0.0f, true);
    delay(2000);

    Serial.println("Motor 2 -5V");
    exp.pinVoltage(5, 8.1f, true);
    exp.pinVoltage(6, 0.0f, true);
    delay(2000);

    Serial.println("Motor 2 2.5V");
    exp.pinVoltage(5, 0.0f, true);
    exp.pinVoltage(6, 5.2f, true);
    delay(2000);

    Serial.println("Motor 2 5V");
    exp.pinVoltage(5, 0.0f, true);
    exp.pinVoltage(6, 8.6f, true);
    delay(2000);

    testDone = true;
  }

  OptaController.update();
}


