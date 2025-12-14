# IBM-7535-Robotic-Arm-ControlSys

[![Watch the demo video](https://img.youtube.com/vi/oRWTlzxxvLM/maxresdefault.jpg)](https://youtu.be/oRWTlzxxvLM)

## Project Overview
**Rewired & ReARMed** is a senior design project focused on retrofitting an **IBM 7535 SCARA robot** from the 1980s with modern control hardware and software.  
The goal was to replace the obsolete control system with an **Arduino Opta PLC** while maintaining the robot’s functionality and old legacy systems.

Originally designed for industrial automation, the IBM 7535 had been rendered non-functional due to outdated electronics, limited documentation, and component issues. Our team reverse-engineered the system, traceed its wiring diagrams, and designed a new control approach compatible with the existing **Yaskawa Servopak motor drivers**. This included:
- developing an **analog ±10V control solution** using op-amp subtractor circuits, enabling bidirectional motor control from the Arduino Opta’s 0–10V outputs
- making a **voltage divider circuit using Zener diodes** to enable bidirectional control for the stepper motor
- successfully pulse the stepper motor at operable speed
- send voltage to operate the solenoids

In addition to wiring the OPTA to the system, we programmed control logic within the Arduino IDE, verified the sensors, and ensured the motors responded accurately to control inputs. The result was a functioning robot capable of smooth and controlled movement, demonstrated successfully in a live test.
While the SCARA is operable and functionable, it still needs future work, like finishing the encoders and tracing a possible overcurrent issue in the servopacks, an issue that is correlated to the motor drift.
Despite the small issues, the team was able to pave a way for future teams to work on the 7535 SCARA and hopes to see the robotic arm be completely restored and retrofitted with the versatility and capability of modern technology.
