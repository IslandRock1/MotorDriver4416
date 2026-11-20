//
// Created by Øystein Bringsli.
//

#include "esp_timer.h"

#include "PrintInfo.h"
#include "MotorController.h"

float PrintInfo::RPM = 0;
uint32_t PrintInfo::lastPrint = 0;

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

void PrintInfo::printDriverTime(uint32_t sumTime) {
    uint32_t currentTime = esp_timer_get_time();

    uint32_t loopTime = currentTime - lastPrint;
    lastPrint = currentTime;

    Serial.print("Driver time: ");
    Serial.print(sumTime);
    Serial.print("/");
    Serial.print(loopTime);
    Serial.print(" => ");
    Serial.print(static_cast<double>(sumTime) / static_cast<double>(loopTime));
    Serial.println("%.");
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


