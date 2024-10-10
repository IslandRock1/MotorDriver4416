
#include <Arduino.h>
#include "PrintInfo.hpp"

// Connect to the two encoder outputs!
#define ENCODER_A   13
#define ENCODER_B   12

#define a1  26
#define a2  25

// PWM settings
const int PWMFreq = 5000;    // Frequency of the PWM signal in Hz
const int PWMResolution = 8; // 8-bit resolution, so duty cycle ranges from 0 to 255
const int PWMChannel1 = 0;
const int PWMChannel2 = 1;

void controlMotorSpeed(int dutyCycle, bool dir);

void IRAM_ATTR interruptA() {

    PrintInfo::pulseCount++;
    /*motordir = digitalRead(ENCODER_B);

    uint32_t currA = micros();
    uint32_t diffA = currA - lastA;
    if (lastA > currA) {
        diffA = (0xFFFFFFFF - lastA) + currA;
    }

    pulseCount++;

    lastA = currA;

    if (pulseCount > 2000) {
        controlMotorSpeed(0, true);
    }*/
}

void setup() {
    Serial.begin(115200);           // set up Serial library at 9600 bps
    Serial.println("MMMMotor party!");

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

    delay(100);

    controlMotorSpeed(150, true);
}

void controlMotorSpeed(int dutyCycle, bool dir) {
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



auto lastPrint = millis();
void loop() {

    if (PrintInfo::pulseCount > 4245) {
        controlMotorSpeed(0, true);
    }

    Serial.println(PrintInfo::pulseCount);


    if (lastPrint + 50 < millis()) {
        PrintInfo::printRPM();
        lastPrint = millis();
    }
}