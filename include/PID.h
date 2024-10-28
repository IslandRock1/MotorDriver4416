//
// Created by oyste on 10/10/2024.
//

#ifndef PID_H
#define PID_H

#include <Arduino.h>

class PID {
public:
	// Constructor to initialize PID gains and limits
	PID(float kp, float ki, float kd, float min_output, float max_output)
		: Kp(kp), Ki(ki), Kd(kd), minOutput(min_output), maxOutput(max_output),
		  prevError(0), integral(0), lastTime(0) {}

	// Method to calculate the control signal
	float calculate(long long int setpoint, long long int measured_value) {
		// Get current time (assuming this is running on a platform with millis())
		unsigned long currentTime = millis();
		float deltaTime = (currentTime - lastTime);  // Time in ms
        deltaTime++; // Very often this becomes 0, so i just add 1. Whatever

		// Calculate error
		float error = setpoint - measured_value;

		// Proportional term
		float Pout = Kp * error;

		// Integral term
		integral += error * deltaTime;

		// if (integral * integralSign < 0) {
		// 	integral = 0;
		//
		// 	if (integralSign == -1) {
		// 		integralSign = 1;
		// 	} else {
		// 		integralSign = -1;
		// 	}
		// }

		float Iout = Ki * integral;

		// Derivative term
		float derivative = (error - prevError) / deltaTime;
		float Dout = Kd * derivative;

		// Combine all the terms
		float output = Pout + Iout + Dout;

		// Constrain output to the defined limits
		if (output > maxOutput) {
			output = maxOutput;
		} else if (output < minOutput) {
			output = minOutput;
		}

		// Store the current error and time for the next iteration
		prevError = error;
		lastTime = currentTime;

		return output;
	}

	// Method to reset the PID (useful between control loops)
	void reset() {
		integral = 0;
		integralSign = -1;
		prevError = 0;
		lastTime = millis();
	}

private:
	float Kp, Ki, Kd;             // PID coefficients
	float minOutput, maxOutput;   // Output constraints
	float prevError;              // Previous error for derivative calculation
	float integral;               // Integral sum
	float integralSign = -1;
	unsigned long lastTime;       // Last time in millis() for delta time calculation
};



#endif //PID_H
