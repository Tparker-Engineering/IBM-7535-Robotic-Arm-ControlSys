#include "OptaBlue.h"

/*Theta1 needs to be slight off home cam roller. Theta2 needs to be slightly off home sensor. Stepper belt needs to be in a safe postion. 
Close air compressor valve on bottom of aircompressor and plug in compressor. PSI for robot is 80PSI do not exceed.
Demo will execute a simulation of pick and place operation.*/

// Relay outputs
#define Z_DOWN_RELAY    0  // RELAY1 (D0)
#define GRIPPER_RELAY   1  // RELAY2 (D1)

// Sensor inputs (core Opta GPIO)
#define Z_UP_PIN        A6  // Z-UP sensor input
#define Z_DOWN_PIN      A2  // Z-DOWN sensor input

bool gripperCycleDone = false;

enum Theta1State {
  THETA1_IDLE,
  THETA1_MOVE_CCW_1,
  THETA1_HOLD_1,
  THETA1_MOVE_CCW_2,
  THETA1_HOLD_2,
  THETA1_TO_OVERRUN,
  THETA1_TO_HOME,
  THETA1_DONE
};

enum Theta2State {
  THETA2_IDLE,
  THETA2_MOVE_CCW_1,
  THETA2_HOLD_1,
  THETA2_MOVE_CCW_2,
  THETA2_HOLD_2,
  THETA2_TO_OVERRUN_POS,
  THETA2_TO_OVERRUN_NEG,
  THETA2_TO_HOME,
  THETA2_DONE
};

Theta1State theta1_state = THETA1_IDLE;
Theta2State theta2_state = THETA2_IDLE;

bool stepper_active = false;
uint32_t stepper_start_time = 0;
uint8_t stepper_channel = 0;


AnalogExpansion aexp;
float lastVoltages[7] = {0};

void safeSetVoltage(uint8_t channel, float voltage) {
  if (lastVoltages[channel] != voltage) {
    aexp.pinVoltage(channel, voltage);
    lastVoltages[channel] = voltage;
  }
}

void spinStepper(uint8_t channel) {
  aexp.setPwm(channel, 1600, 25);
  OptaController.update();
  delay(250);
  aexp.setPwm(channel, 1600, 0);
  OptaController.update();
}
/*
void runGripperSequence() {
  // Z-UP pull-up hack
  pinMode(Z_UP_PIN, OUTPUT); digitalWrite(Z_UP_PIN, HIGH); delay(1);
  pinMode(Z_UP_PIN, INPUT); delayMicroseconds(5);
  bool zUpActive = (digitalRead(Z_UP_PIN) == LOW);

  // === Software pull-up hack for Z-DOWN (A2) ===
  pinMode(Z_DOWN_PIN, OUTPUT);
  digitalWrite(Z_DOWN_PIN, HIGH);
  delay(1);
  pinMode(Z_DOWN_PIN, INPUT);
  delayMicroseconds(5);
  bool zDownActive = (digitalRead(Z_DOWN_PIN) == LOW);  // active-low

  digitalWrite(Z_DOWN_RELAY, HIGH);  // Lower Z
  delay(1000);
  digitalWrite(Z_DOWN_RELAY, LOW);  // Raise Z
  delay(200);

/*
  if (zUpActive) {
    digitalWrite(Z_DOWN_RELAY, HIGH);  // Lower Z
    delay(500);

    unsigned long startTime = millis();
    while ((millis() - startTime) < 5000) {
      // Z-DOWN pull-up hack
      pinMode(Z_DOWN_PIN, OUTPUT); digitalWrite(Z_DOWN_PIN, HIGH); delay(1);
      pinMode(Z_DOWN_PIN, INPUT); delayMicroseconds(5);
      bool zDownActive = (digitalRead(Z_DOWN_PIN) == LOW);

      if (zDownActive) {
        digitalWrite(GRIPPER_RELAY, HIGH);  // Grip
        delay(1000);
        digitalWrite(GRIPPER_RELAY, LOW);
        break;
      }
    }

    digitalWrite(Z_DOWN_RELAY, LOW);  // Raise Z
  }
  
  delay(200);
}
*/

// Timing parameters
const unsigned long theta1_moveDuration1 = 4000;
const unsigned long theta1_moveDuration2 = 2000;
const unsigned long theta1_holdDuration = 4000;
unsigned long theta1_timer = 0;

const unsigned long theta2_moveDuration1 = 1250;
const unsigned long theta2_moveDuration2 = 500;
const unsigned long theta2_holdDuration1 = 8000;
const unsigned long theta2_holdDuration2 = 4000;
unsigned long theta2_timer = 0;

void setup() {
  Serial.begin(115200);
  OptaController.begin();

  pinMode(A0, INPUT_PULLUP);  // θ₁ home
  pinMode(A1, INPUT_PULLUP);  // θ₁ overrun
  pinMode(A3, INPUT_PULLUP);  // θ₂ home
  pinMode(A4, INPUT_PULLUP);  // θ₂ overrun−
  pinMode(A5, INPUT_PULLUP);  // θ₂ overrun+
  // Set relay outputs
  pinMode(Z_DOWN_RELAY, OUTPUT);
  pinMode(GRIPPER_RELAY, OUTPUT);
  digitalWrite(Z_DOWN_RELAY, LOW);
  digitalWrite(GRIPPER_RELAY, LOW);


  while (OptaController.getExpansionNum() == 0) {
    Serial.println("Waiting for analog expansion...");
    OptaController.update();
  }

  aexp = OptaController.getExpansion(0);

  aexp.beginChannelAsDac(2, OA_VOLTAGE_DAC, true, false, OA_SLEW_RATE_0);  // θ₁ CW
  aexp.beginChannelAsDac(3, OA_VOLTAGE_DAC, true, false, OA_SLEW_RATE_0);  // θ₁ CCW
  aexp.beginChannelAsDac(5, OA_VOLTAGE_DAC, true, false, OA_SLEW_RATE_0);  // θ₂ CW
  aexp.beginChannelAsDac(6, OA_VOLTAGE_DAC, true, false, OA_SLEW_RATE_0);  // θ₂ CCW

  aexp.setPwm(OA_PWM_CH_0, 1600, 0);  // Stepper CCW idle
  aexp.setPwm(OA_PWM_CH_1, 1600, 0);  // Stepper CW idle

  safeSetVoltage(2, 0.0f);
  safeSetVoltage(3, 0.4f);
  safeSetVoltage(5, 0.0f);
  safeSetVoltage(6, 0.4f);

  OptaController.update();
  Serial.println("Setup complete.");
}

void loop() {

  // θ₁ sensor reads
  pinMode(A0, OUTPUT); digitalWrite(A0, HIGH); delayMicroseconds(10);
  pinMode(A0, INPUT); delayMicroseconds(5);
  bool theta1_home = (digitalRead(A0) == LOW);

  pinMode(A1, OUTPUT); digitalWrite(A1, HIGH); delayMicroseconds(10);
  pinMode(A1, INPUT); delayMicroseconds(5);
  bool theta1_overrun = (digitalRead(A1) == LOW);

  // θ₂ sensor reads
  bool theta2_home = (digitalRead(A3) == LOW);
  bool theta2_overrunNeg = (digitalRead(A4) == LOW);
  bool theta2_overrunPos = (digitalRead(A5) == LOW);

  bool zDownActive = (digitalRead(Z_DOWN_PIN) == LOW);  // active-low
  bool zUpActive = (digitalRead(Z_UP_PIN) == LOW);


  OptaController.update();

  // θ₁ State Machine
  switch (theta1_state) {
    case THETA1_IDLE:
      safeSetVoltage(2, 0.0f);
      safeSetVoltage(3, 0.4f);
      if (theta1_home) {
        theta1_timer = millis();
        Serial.println("θ₁ → MOVE_CCW_1");
        theta1_state = THETA1_MOVE_CCW_1;
      }
      break;

    case THETA1_MOVE_CCW_1:
      safeSetVoltage(2, 0.0f);
      safeSetVoltage(3, 4.5f);
      if (millis() - theta1_timer >= theta1_moveDuration1) {
        safeSetVoltage(3, 0.0f);
        theta1_timer = millis();
        Serial.println("θ₁ → HOLD_1");
        theta1_state = THETA1_HOLD_1;
      }
      break;

    case THETA1_HOLD_1:
      safeSetVoltage(2, 0.0f);
      safeSetVoltage(3, 0.4f);
        
        aexp.setPwm(OA_PWM_CH_0, 1600, 25);  // start CCW
        OptaController.update();
        delay(250);
        aexp.setPwm(OA_PWM_CH_0, 1600, 1);  // almost 0% duty
        OptaController.update();
        delay(2);                               // allow hardware to register
        aexp.setPwm(OA_PWM_CH_0, 1600, 0);  // now fully stop
        OptaController.update();
        //runGripperSequence();

        delay(400);
        // Z-UP pull-up hack
        pinMode(Z_UP_PIN, OUTPUT); digitalWrite(Z_UP_PIN, HIGH); delay(1);
        pinMode(Z_UP_PIN, INPUT); delayMicroseconds(5);

        // === Software pull-up hack for Z-DOWN (A2) ===
        pinMode(Z_DOWN_PIN, OUTPUT);
        digitalWrite(Z_DOWN_PIN, HIGH);
        delay(1);
        pinMode(Z_DOWN_PIN, INPUT);
        delayMicroseconds(5);


        digitalWrite(Z_DOWN_RELAY, HIGH);  // Lower Z
        delay(1000);
        digitalWrite(Z_DOWN_RELAY, LOW);  // Raise Z
        delay(1000);


        delay(4000);
      /*
      if (stepper_active && millis() - stepper_start_time <= 300) {
        aexp.setPwm(stepper_channel, 1600, 1);  // almost 0% duty
        delay(2);                               // allow hardware to register
        aexp.setPwm(stepper_channel, 1600, 0);  // now fully stop
        OptaController.update();                // apply to hardware
        OptaController.update();
        stepper_active = false;
      }
      
      //spinStepper(OA_PWM_CH_0);  // For CCW
      if (!stepper_active) {
        aexp.setPwm(OA_PWM_CH_0, 1600, 25);  // start CCW
        OptaController.update();
        stepper_channel = OA_PWM_CH_0;
        stepper_start_time = millis();
        stepper_active = true;
        Serial.println("θ₁: Spinning stepper CCW");
      }
      
      */
      // Serial.println("θ₁: Spinning stepper CCW");
      if (millis() - theta1_timer >= theta1_holdDuration) {
        theta1_timer = millis();
        Serial.println("θ₁ → MOVE_CCW_2");
        theta1_state = THETA1_MOVE_CCW_2;
      }
      break;

    case THETA1_MOVE_CCW_2:
      safeSetVoltage(2, 0.0f);
      safeSetVoltage(3, 4.5f);
      if (millis() - theta1_timer >= theta1_moveDuration2) {
        safeSetVoltage(3, 0.0f);
        theta1_timer = millis();
        Serial.println("θ₁ → HOLD_2");
        theta1_state = THETA1_HOLD_2;
      }
      break;

    case THETA1_HOLD_2:
      safeSetVoltage(2, 0.0f);
      safeSetVoltage(3, 0.4f);
      // spinStepper(OA_PWM_CH_1);  // For CW

      delay(400);
      // Z-UP pull-up hack
      if (!gripperCycleDone) {
        delay(400);

        // Pull-up hack for Z-UP
        pinMode(Z_UP_PIN, OUTPUT); digitalWrite(Z_UP_PIN, HIGH); delay(1);
        pinMode(Z_UP_PIN, INPUT); delayMicroseconds(5);

        // Pull-up hack for Z-DOWN
        pinMode(Z_DOWN_PIN, OUTPUT);
        digitalWrite(Z_DOWN_PIN, HIGH);
        delay(1);
        pinMode(Z_DOWN_PIN, INPUT);
        delayMicroseconds(5);

        Serial.println("Z lowering...");
        digitalWrite(Z_DOWN_RELAY, HIGH);
        delay(1000);
        Serial.println("Z raising...");
        digitalWrite(Z_DOWN_RELAY, LOW);
        delay(1000);

        gripperCycleDone = true;
      }


      //runGripperSequence();

        /*
        aexp.setPwm(OA_PWM_CH_1, 1600, 25);  // start CCW
        OptaController.update();
        delay(250);
        aexp.setPwm(OA_PWM_CH_1, 1600, 1);  // almost 0% duty
        OptaController.update();
        delay(2);                               // allow hardware to register
        aexp.setPwm(OA_PWM_CH_1, 1600, 0);  // now fully stop
        OptaController.update();
        */

      /*
      if (!stepper_active) {
        aexp.setPwm(OA_PWM_CH_1, 1600, 25);  // start CCW
        OptaController.update();
        stepper_channel = OA_PWM_CH_0;
        stepper_start_time = millis();
        stepper_active = true;
        Serial.println("θ₁: Spinning stepper CCW");
      }
      */

      if (millis() - theta1_timer >= theta1_holdDuration) {
        Serial.println("θ₁: Spinning stepper CW");
        Serial.println("θ₁ → TO_OVERRUN");
        safeSetVoltage(3, 4.5f);

        theta1_state = THETA1_TO_OVERRUN;
      }
      break;

    case THETA1_TO_OVERRUN:
      if (theta1_overrun) {
        Serial.println("θ₁ overrun detected. Returning home.");
        safeSetVoltage(2, 5.2f);
        safeSetVoltage(3, 0.0f);
        theta1_state = THETA1_TO_HOME;
      }
      break;

    case THETA1_TO_HOME:
      if (theta1_home) {
        Serial.println("θ₁ returned to home.");
        safeSetVoltage(2, 0.0f);
        safeSetVoltage(3, 0.0f);
        theta1_state = THETA1_DONE;
      }
      break;

    case THETA1_DONE:
      // No action
      break;
  }

  // θ₂ State Machine
  switch (theta2_state) {
    case THETA2_IDLE:
      safeSetVoltage(5, 0.0f);
      safeSetVoltage(6, 0.4f);
      if (theta2_home) {
        theta2_timer = millis();
        Serial.println("θ₂ → MOVE_CCW_1");
        theta2_state = THETA2_MOVE_CCW_1;
      }
      break;

    case THETA2_MOVE_CCW_1:
      safeSetVoltage(5, 0.0f);
      safeSetVoltage(6, 4.5f);
      if (millis() - theta2_timer >= theta2_moveDuration1) {
        safeSetVoltage(6, 0.0f);
        theta2_timer = millis();
        Serial.println("θ₂ → HOLD_1");
        theta2_state = THETA2_HOLD_1;
      }
      break;

    case THETA2_HOLD_1:
      safeSetVoltage(5, 0.0f);
      safeSetVoltage(6, 0.4f);
      if (millis() - theta2_timer >= theta2_holdDuration1) {
        theta2_timer = millis();
        Serial.println("θ₂ → MOVE_CCW_2");
        theta2_state = THETA2_MOVE_CCW_2;
      }
      break;

    case THETA2_MOVE_CCW_2:
      safeSetVoltage(5, 0.0f);
      safeSetVoltage(6, 4.5f);
      if (millis() - theta2_timer >= theta2_moveDuration2) {
        safeSetVoltage(6, 0.0f);
        theta2_timer = millis();
        Serial.println("θ₂ → HOLD_2");
        theta2_state = THETA2_HOLD_2;
      }
      break;

    case THETA2_HOLD_2:
      safeSetVoltage(5, 0.0f);
      safeSetVoltage(6, 0.4f);
      if (millis() - theta2_timer >= theta2_holdDuration2) {
        Serial.println("θ₂ → TO_OVERRUN_POS");
        safeSetVoltage(6, 4.5f);
        theta2_state = THETA2_TO_OVERRUN_POS;
      }
      break;

    case THETA2_TO_OVERRUN_POS:
      if (theta2_overrunPos) {
        Serial.println("θ₂ → TO_OVERRUN_NEG");
        safeSetVoltage(5, 5.2f);
        safeSetVoltage(6, 0.0f);
        theta2_state = THETA2_TO_OVERRUN_NEG;
      }
      break;

    case THETA2_TO_OVERRUN_NEG:
      if (theta2_overrunNeg) {
        Serial.println("θ₂ → TO_HOME");
        safeSetVoltage(5, 0.0f);
        safeSetVoltage(6, 4.5f);
        theta2_state = THETA2_TO_HOME;
      }
      break;

    case THETA2_TO_HOME:
      if (theta2_home) {
        Serial.println("θ₂ returned to home.");
        safeSetVoltage(5, 0.0f);
        safeSetVoltage(6, 0.0f);
        theta2_state = THETA2_DONE;
      }
      break;

    case THETA2_DONE:
      // No action
      break;
  }

  delay(20);
}


