#include "OptaBlue.h"

/*Close air compressor valve on bottom of aircompressor and plug in compressor. PSI for robot is 80PSI do not exceed.*/

// Relay outputs
#define Z_DOWN_RELAY    0  // RELAY1 (D0)
#define GRIPPER_RELAY   1  // RELAY2 (D1)

// Sensor inputs (core Opta GPIO)
#define Z_UP_PIN        A6  // Z-UP sensor input
#define Z_DOWN_PIN      A2  // Z-DOWN sensor input

void setup() {
  Serial.begin(115200);
  delay(1000);

  OptaController.begin();

  // Set relay outputs
  pinMode(Z_DOWN_RELAY, OUTPUT);
  pinMode(GRIPPER_RELAY, OUTPUT);
  digitalWrite(Z_DOWN_RELAY, LOW);
  digitalWrite(GRIPPER_RELAY, LOW);

  Serial.println("Setup complete. Monitoring sensors on A6 (Z-UP), A2 (Z-DOWN).");
}

void loop() {
  // === Software pull-up hack for Z-UP (A6) ===
  pinMode(Z_UP_PIN, OUTPUT);
  digitalWrite(Z_UP_PIN, HIGH);
  delay(1);  // Give time to charge line
  pinMode(Z_UP_PIN, INPUT);
  delayMicroseconds(5);
  bool zUpActive = (digitalRead(Z_UP_PIN) == LOW);  // active-low

  // === Software pull-up hack for Z-DOWN (A2) ===
  pinMode(Z_DOWN_PIN, OUTPUT);
  digitalWrite(Z_DOWN_PIN, HIGH);
  delay(1);
  pinMode(Z_DOWN_PIN, INPUT);
  delayMicroseconds(5);
  bool zDownActive = (digitalRead(Z_DOWN_PIN) == LOW);  // active-low

  // Debug print
  Serial.print("Z-UP (A6): ");
  Serial.print(zUpActive ? "ACTIVE" : "inactive");
  Serial.print(" | Z-DOWN (A2): ");
  Serial.println(zDownActive ? "ACTIVE" : "inactive");

  // === Main logic ===
  if (zUpActive) {
    Serial.println("Z-UP detected. Lowering Z...");

    digitalWrite(Z_DOWN_RELAY, HIGH);  // Engage Z-DOWN
    delay(500);  // Give time for motion to start

    unsigned long startTime = millis();
    while ((millis() - startTime) < 5000) {
      // Refresh Z-DOWN sensor pull-up hack
      pinMode(Z_DOWN_PIN, OUTPUT);
      digitalWrite(Z_DOWN_PIN, HIGH);
      delay(1);
      pinMode(Z_DOWN_PIN, INPUT);
      delayMicroseconds(5);
      zDownActive = (digitalRead(Z_DOWN_PIN) == LOW);

      if (zDownActive) {
        Serial.println("Z-DOWN detected. Activating gripper...");
        digitalWrite(GRIPPER_RELAY, HIGH);
        delay(1000);  // Grip
        digitalWrite(GRIPPER_RELAY, LOW);
        break;
      }
    }

    digitalWrite(Z_DOWN_RELAY, LOW);  // Always stop Z after sequence
  }

  delay(200);  // Polling interval
}
