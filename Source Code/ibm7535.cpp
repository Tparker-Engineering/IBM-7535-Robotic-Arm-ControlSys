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


#include "IBM7535.h"

// Initializes the control system
void initIBM7535 ()
{
	Serial.begin(115200);
	delay(1000);
	
	OptaController.begin();
	
	pinMode(A0, INPUT_PULLUP);  // I1 - θ₁ home
	pinMode(A1, INPUT_PULLUP);  // I2 - θ₁ overrun (shared)
	pinMode(A3, INPUT_PULLUP);  // I4 - θ₂ home
	pinMode(A4, INPUT_PULLUP);  // I5 - θ₂ overrun+
	pinMode(A5, INPUT_PULLUP);  // I6 - θ₂ overrun−
	
	
	
	
	while (OptaController.getExpansionNum() < 3) 
	{
		Serial.println("Waiting for expansions...");
		OptaController.update();
		delay(500);
	}
	
	AnalogExpansion aexp = OptaController.getExpansion(0);
	DigitalExpansion dexp = OptaController.getExpansion(1);
	
	if (aexp)
	{
		aexp.beginChannelAsDac(2, OA_VOLTAGE_DAC, true, false, OA_SLEW_RATE_0);
		aexp.beginChannelAsDac(3, OA_VOLTAGE_DAC, true, false, OA_SLEW_RATE_0);
		aexp.beginChannelAsDac(5, OA_VOLTAGE_DAC, true, false, OA_SLEW_RATE_0);
		aexp.beginChannelAsDac(6, OA_VOLTAGE_DAC, true, false, OA_SLEW_RATE_0);
		
		aexp.setPwm(OA_PWM_CH_0, STEPPER_PERIOD, 0);
		aexp.setPwm(OA_PWM_CH_1, STEPPER_PERIOD, 0);
		
		aexp.pinVoltage(5, 0.0f);
		aexp.pinVoltage(6, 0.0f);
		aexp.pinVoltage(2, 0.0f);
		aexp.pinVoltage(3, 0.0f);
	}
	
	if (dexp) 
	{
		
	}
	
	
	OptaController.update();
}


void theta1CW (AnalogExpansion aexp, float voltage)
{
	float vmap;
	
	// Values here determined via linear regression of CH0 data.
	// Should approximately correspond to the proper output voltage.
	vmap = (voltage + CH0_INT) / CH0_SLOPE;
	
	if (vmap > OPTA_DAC_MAX) vmap = OPTA_DAC_MAX;
	
	aexp.pinVoltage(2, 0.0f, true); 
	aexp.pinVoltage((3, vmap, true); 
}
void theta1CCW (AnalogExpansion* aexp, float voltage)
{
	float vmap;
	
	// Values here determined via linear regression of CH1 data.
	// Should approximately correspond to the proper output voltage.
	vmap = (voltage + CH1_INT) / CH1_SLOPE;
	
	if (vmap > OPTA_DAC_MAX) vmap = OPTA_DAC_MAX;
	
	aexp.pinVoltage((2, vmap, true); 
	aexp.pinVoltage((3, 0.0f, true); 
}
void theta2CW (AnalogExpansion aexp, float voltage)
{
	// vmap represents the actual output voltage needed from the Opta
	// to be able to generate the reference voltage specifed.
	float vmap;
	
	// Values here determined via linear regression of CH0 data.
	// Should approximately correspond to the proper output voltage.
	vmap = (voltage + CH0_INT) / CH0_SLOPE;
	
	if (vmap > OPTA_DAC_MAX) vmap = OPTA_DAC_MAX;
	
	aexp.pinVoltage(5, 0.0f, true); 
	aexp.pinVoltage(6, vmap, true); 
}
void theta2CCW (AnalogExpansion* aexp, float voltage)
{
	float vmap;
	
	// Values here determined via linear regression of CH1 data.
	// Should approximately correspond to the proper output voltage.
	vmap = (voltage + CH1_INT) / CH1_SLOPE;
	
	if (vmap > OPTA_DAC_MAX) vmap = OPTA_DAC_MAX;
	
	aexp.pinVoltage(5, vmap, true); 
	aexp.pinVoltage(6, 0.0f, true); 
}


void zUp (AnalogExpansion aexp)
{
	/* checks if the PNs are in the down position
	 * there are two separate lim sw for the PNs, different feedback
	 * all these PN fns should follow the same formula
	 * wait, is there lim sw's for the gripper?
	if (!up && down)
		24v to solenoid
	else
		do nothing 
	*/
}

void zDown(AnalogExpansion aexp)
{
	
}

void gripperOpen(AnalogExpansion aexp)
{
	
}

void gripperClose(AnalogExpansion aexp)
{
	
}


// Rotates the roll stepper motor in the clockwise direction
void rollCW (AnalogExpansion aexp, uint32_t pulse)
{
	// Because of the stepper card's hardware quirks, direction is 
	// controlled by separate PWM lines, which means* that the channel
	// corresponding to the opposite direction should be zero'd out
	// before use.
	//
	// *At least, we didn't test this and it's probably not a good idea.
	aexp.setPwm(OA_PWM_CH_1, STEPPER_PERIOD, 0);
	
	// STEPPER_PERIOD is defined in the header and should always be <2.5kHz
	aexp.setPwm(OA_PWM_CH_0, STEPPER_PERIOD, pulse);
}

// Rotates the roll stepper motor in the counterclockwise direction
void rollCCW (AnalogExpansion aexp, uint32_t pulse)
{
	aexp.setPwm(OA_PWM_CH_0, STEPPER_PERIOD, 0);
	aexp.setPwm(OA_PWM_CH_1, STEPPER_PERIOD, pulse);
}