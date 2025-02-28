//
// Created by oyste on 10/10/2024.
//

#include "MotorController.h"

void IRAM_ATTR interruptA() {
    MotorController::motordir = digitalRead(ENCODER_B);
    MotorController::currentPosition -= MotorController::motordir * 2 - 1;
}

volatile bool MotorController::motordir = false;
volatile long long int MotorController::currentPosition = INT32_MAX / 2;
MotorController::MotorController() {


    pinMode(ENCODER_B, INPUT_PULLUP);
    pinMode(ENCODER_A, INPUT_PULLUP);

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

void MotorController::initInterrupt() {
    attachInterrupt(digitalPinToInterrupt(ENCODER_A), interruptA, RISING);
}

void MotorController::controlMotorSpeed(int dutyCycle, bool dir) const {
    dutyCycle = constrain(dutyCycle, 0, 255);

    if (dir) {
        ledcWrite(PWMChannel1, 0);
        ledcWrite(PWMChannel2, dutyCycle);
    } else {
        ledcWrite(PWMChannel1, dutyCycle);
        ledcWrite(PWMChannel2, 0);
    }
}

void MotorController::SetPosition(long int position) {
    desiredPosition = 2147483648 + position;
}

void MotorController::SetPositionAbsolute(unsigned long int position) {
    desiredPosition = position;
}

void MotorController::incrementPositionRounds(long int rounds) {
    desiredPosition += rounds * encoderAndGearingForMotor;
}

void MotorController::decrementPositionRounds(long int rounds) {
    desiredPosition -= rounds * encoderAndGearingForMotor;
}

void MotorController::incrementPositionRadians(unsigned long int radians) {
    desiredPosition += radians * encoderAndGearingForMotor / TWO_PI;
}

void MotorController::decrementPositionRadians(unsigned long int radians) {
    desiredPosition -= radians * encoderAndGearingForMotor / TWO_PI;
}

void MotorController::incrementPositionPulsecount(unsigned long int increment) {
    desiredPosition += increment;
}

void MotorController::decrementPositionPulsecount(unsigned long int decrement) {
    desiredPosition -= decrement;
}

void MotorController::updatePosition() {

    auto out = pid.calculate(desiredPosition, currentPosition);

    const auto outInt = static_cast<int>(out);

    if (out < 0) {
        controlMotorSpeed(outInt * -1, true);
    } else {
        controlMotorSpeed(outInt, false);
    }
}
