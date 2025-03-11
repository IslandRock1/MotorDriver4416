//
// Created by oyste on 10/10/2024.
//

#ifndef MOTORCONTROLLER_H
#define MOTORCONTROLLER_H

#include <Arduino.h>
#include "PID.h"

#define ENCODER_A   13
#define ENCODER_B   18

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

	const double encoderAndGearingForMotor = 13.0 * 30613.0 / 1500.0;

    static volatile bool motordir;
	static volatile long long int currentPosition;
	long long int desiredPosition = INT32_MAX / 2;

    static volatile uint32_t sumTime;

	PID pid = PID(100.0, 0.0, 0.0, -255.0, 255.0);

    void initInterrupt();

	void controlMotorSpeed(int dutyCycle, bool dir) const;

	void SetPosition(long int position);
	void SetPositionAbsolute(unsigned long int position);

	void incrementPositionRounds(long int rounds);
	void decrementPositionRounds(long int rounds);

	void incrementPositionRadians(unsigned long int radians);
	void decrementPositionRadians(unsigned long int radians);

	void incrementPositionPulsecount(unsigned long int increment);
	void decrementPositionPulsecount(unsigned long int decrement);

	void updatePosition();
};



#endif //MOTORCONTROLLER_H
