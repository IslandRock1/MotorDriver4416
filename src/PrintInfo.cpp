//
// Created by Øystein Bringsli.
//

#include "PrintInfo.h"
#include "MotorController.h"

float PrintInfo::RPM = 0;

void PrintInfo::printRPM(unsigned long int desiredPosition) {

    calculateRPM();

    Serial.print("Direction: ");
    if (MotorController::motordir) {
        Serial.print("CW @ ");
    } else {
        Serial.print("CCW @ ");
    }
    Serial.print(static_cast<int>(RPM));
    Serial.print(" RPM. Pulsecount: ");
    Serial.print(MotorController::currentPosition);
    Serial.print("/");
    Serial.print(desiredPosition);
    Serial.print(". Error: ");
    Serial.println(desiredPosition - MotorController::currentPosition);
}

void PrintInfo::calculateRPM() {
    static uint32_t lastPulseCount = 0;
    static uint32_t lastCalcTime = 0;

    uint32_t currentTime = millis();
    uint32_t deltaTime = currentTime - lastCalcTime;

    uint32_t deltaPulses = MotorController::currentPosition - lastPulseCount;

    // Calculate RPM
    double revs = deltaPulses;
    revs /= (gearing * encoderTicks);
    revs *= (60000.0 / deltaTime);

    RPM = revs;

    // Update for next calculation
    lastPulseCount = MotorController::currentPosition;
    lastCalcTime = currentTime;

}


