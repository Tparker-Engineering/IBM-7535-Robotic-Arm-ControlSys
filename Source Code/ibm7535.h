// _____  ______   ______     _______  _______  ________ _______ 
//(_____)(____  \ |  ___ \   (_______)(_______)(_______/(_______)
//   _    ____)  )| | _ | |        _   ______     ____   ______  
//  | |  |  __  ( | || || |       / ) (_____ \   (___ \ (_____ \ 
// _| |_ | |__)  )| || || |      / /   _____) ) _____) ) _____) )
//(_____)|______/ |_||_||_|     (_/   (______/ (______/ (______/ 
//                                                               
//     ______                                          _   
//    (_____ \                                        | |  
//     _____) )  ____   ____   ____  ____    ____   _ | |  
//    (_____ (  / _  ) / _  | / ___)|    \  / _  ) / || |  
//          | |( (/ / ( ( | || |    | | | |( (/ / ( (_| |  
//          |_| \____) \_||_||_|    |_|_|_| \____) \____|  
//                                     _                   
//                                    | |                  
//                    ____  ____    _ | |                  
//                   / _  ||  _ \  / || |                  
//                  ( ( | || | | |( (_| |                  
//                   \_||_||_| |_| \____|                  
//    ______                 _                    _       
//    (_____ \               (_)                  | |      
//     _____) )  ____  _ _ _  _   ____   ____   _ | |      
//    (_____ (  / _  )| | | || | / ___) / _  ) / || |      
//          | |( (/ / | | | || || |    ( (/ / ( (_| |      
//          |_| \____) \____||_||_|     \____) \____|      
//
// splash created using the Text to ASCII Art Generator located at
// patorjk.com/software/taag/
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Herein describes a very basic library for using the IBM 7535 Robot Arm
// This library provides functions for simple movement on each of its
// four axes.
//
// Due to time and resource constraints, specific functions for a finer
// tuned control over position, with full usage of the sensors and 
// switches, is not supported at present. 
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// CREDITS
// Library Author: Xander Addington
// Original Test Code: Travis Parker
// OptaBlue Library created by Arduino
// 
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// HARDWARE INFO
//
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#ifndef _IBM7535_H_
#define _IBM7535_H_

#include "../Arduino_Opta_Blueprint/src/Optablue.h"

// PWM period for the roll axis
// 1 / 400 us = 2.5 kHz
#define STEPPER_PERIOD 1600

// Maximum output voltage for the analog expansion DACs is 11 volts
#define OPTA_DAC_MAX 11.0

// Transfer function values to convert analog expansion voltage outputs
// to the requested reference voltage for the servo packs
// Derived from a linear regression on CH0 data
#define CH0_SLOPE 0.5806
#define CH0_INT 0.2119
// Derived from a linear regression on CH1 data
#define CH1_SLOPE 0.6028
#define CH1_INT -0.01526
// Linear regressions were done at
// https://www.graphpad.com/quickcalcs/linear1/
// because my calculator can't do that

// Initializes the control system
void initIBM7535 ();

// For the Theta 1 and 2 motors, the control system provides a voltage
// reference to the servo packs that determines the speed and direction
// of the motors.
// The servo packs do the heavy lifting for these functions, providing
// the PWM signals and handling some of the motor feedback. 
// The voltage parameter is the desired reference voltage for the 
// servo packs, and has a range of 0.0 to 5.0 (volts)
// Counter Clockwise (CCW) represents is the positive direction
// Clockwise (CW) represents the negative direction
void theta1CW (AnalogExpansion aexp, float voltage);
void theta1CCW (AnalogExpansion aexp, float voltage);
void theta2CW (AnalogExpansion aexp, float voltage);
void theta2CCW (AnalogExpansion aexp, float voltage);


// The Z axis and gripper attachment are both controlled by simple solenoids
// driven by a 24 volt trigger.
// The gripper attachment is not included with our IBM7535 arm at time of 
// this writing.
void zUp (AnalogExpansion aexp);
void zDown(AnalogExpansion aexp);
void gripperOpen(AnalogExpansion aexp);
void gripperClose(AnalogExpansion aexp);


// The roll axis is driven by a stepper motor connected to a separate 
// stepper card that accepts a two PWM signals from the main controller,
// one for each rotational direction.
// These functions provide the duty cycle for the PWM signals.
void rollCW (AnalogExpansion aexp, uint32_t pulse);
void rollCCW (AnalogExpansion aexp, uint32_t pulse);


// 
// 
bool getTheta2OverrunPos (AnalogExpansion aexp);
bool getTheta2OverrunNeg (AnalogExpansion aexp);
bool getTheta2Home (AnalogExpansion aexp);

bool getTheta1OverrunPos (AnalogExpansion aexp);
bool getTheta1OverrunNeg (AnalogExpansion aexp);
bool getTheta1Home (AnalogExpansion aexp);

float getTheta1Position (AnalogExpansion aexp);
float getTheta2Position (AnalogExpansion aexp);

bool getRollHome (AnalogExpansion aexp);

bool getZUp (AnalogExpansion aexp);
bool getZDown (AnalogExpansion aexp);


#endif