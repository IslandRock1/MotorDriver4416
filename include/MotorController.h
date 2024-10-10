//
// Created by oyste on 10/10/2024.
//

#ifndef MOTORCONTROLLER_H
#define MOTORCONTROLLER_H

#include <Arduino.h>
#include "PID.h"

#define ENCODER_A   13
#define ENCODER_B   12

#define a1  26
#define a2  25

void IRAM_ATTR interruptA();

class MotorController {
public:
	MotorController();

	// PWM settings
	const int PWMFreq = 5000;    // Frequency of the PWM signal in Hz
	const int PWMResolution = 8; // 8-bit resolution, so duty cycle ranges from 0 to 255
	const int PWMChannel1 = 0;
	const int PWMChannel2 = 1;

	static volatile unsigned long int currentPosition;
	unsigned long int desiredPosition = 0;

	PID pid = PID(1.0, 0.0, 0.0, -255.0, 255.0);

	void controlMotorSpeed(int dutyCycle, bool dir);
	void SetPosition(unsigned long int position);
	void updatePosition();
};



#endif //MOTORCONTROLLER_H
