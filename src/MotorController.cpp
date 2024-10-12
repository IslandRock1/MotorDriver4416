//
// Created by oyste on 10/10/2024.
//

#include "MotorController.h"

void IRAM_ATTR interruptA() {
    auto motordir = digitalRead(ENCODER_B);
    MotorController::currentPosition += motordir * 2 - 1;
}

volatile unsigned long int MotorController::currentPosition = 2147483648; // UINT_MAX / 2
MotorController::MotorController() {

    pinMode(ENCODER_B, INPUT_PULLUP);
    pinMode(ENCODER_A, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ENCODER_A), interruptA, RISING);

    // Configure PWM for a1 and a2
    ledcSetup(PWMChannel1, PWMFreq, PWMResolution); // Set up PWM for channel 1
    ledcSetup(PWMChannel2, PWMFreq, PWMResolution); // Set up PWM for channel 2

    // Attach PWM channels to GPIO pins
    ledcAttachPin(a1, PWMChannel1);
    ledcAttachPin(a2, PWMChannel2);

    // Start with motor stopped (duty cycle = 0)
    ledcWrite(PWMChannel1, 0); // Set initial duty cycle to 0 (stopped)
    ledcWrite(PWMChannel2, 0); // Set initial duty cycle to 0 (stopped)
}

void MotorController::controlMotorSpeed(int dutyCycle, bool dir) {
    // Ensure dutyCycle is within the range of 0 to 255
    dutyCycle = constrain(dutyCycle, 0, 255);

    // Apply PWM to the appropriate pins for motor control
    if (dir) {
        // If motor direction is CCW
        ledcWrite(PWMChannel1, 0);      // a1 LOW
        ledcWrite(PWMChannel2, dutyCycle); // a2 PWM control
    } else {
        // If motor direction is CW
        ledcWrite(PWMChannel1, dutyCycle); // a1 PWM control
        ledcWrite(PWMChannel2, 0);      // a2 LOW
    }
}

void MotorController::SetPosition(long int position) {
    desiredPosition = UINT_MAX / 2 + position;
}

void MotorController::SetPositionAbsolute(unsigned long int position) {
    desiredPosition = position;
}

void MotorController::incrementPositionRounds(long int rounds) {
    desiredPosition += rounds * encoderAndGearing;
}

void MotorController::decrementPositionRounds(long int rounds) {
    desiredPosition -= rounds * encoderAndGearing;
}

void MotorController::incrementPositionRadians(unsigned long int radians) {
    desiredPosition += radians * encoderAndGearing / TWO_PI;
}

void MotorController::decrementPositionRadians(unsigned long int radians) {
    desiredPosition -= radians * encoderAndGearing / TWO_PI;
}

void MotorController::incrementPositionPulsecount(unsigned long int increment) {
    desiredPosition += increment;
}

void MotorController::decrementPositionPulsecount(unsigned long int decrement) {
    desiredPosition -= decrement;
}

void MotorController::updatePosition() {
    const auto out = pid.calculate(desiredPosition, currentPosition);
    const auto outInt = static_cast<int>(out);

    if (out < 0) {
        controlMotorSpeed(outInt * -1, false);
    } else {
        controlMotorSpeed(outInt, true);
    }
}
