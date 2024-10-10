//
// Created by Øystein Bringsli.
//

#include "PrintInfo.h"


float PrintInfo::RPM = 0;
volatile bool PrintInfo::motordir = false;
volatile uint32_t PrintInfo::pulseCount = 0;

void PrintInfo::printRPM() {

    calculateRPM();

    Serial.print("Direction: ");
    if (motordir) {
        Serial.print("CW @ ");
    } else {
        Serial.print("CCW @ ");
    }
    Serial.print(static_cast<int>(RPM)); Serial.println(" RPM");
}

void PrintInfo::calculateRPM() {
    static uint32_t lastPulseCount = 0;
    static uint32_t lastCalcTime = 0;

    uint32_t currentTime = millis();
    uint32_t deltaTime = currentTime - lastCalcTime;

    if (deltaTime >= 1000) {  // Calculate every 1 second
        uint32_t deltaPulses = pulseCount - lastPulseCount;

        // Calculate RPM
        double revs = deltaPulses;
        revs /= gearing;                // account for gear ratio
        revs /= encoderTicks;            // account for multiple ticks per rotation
        revs *= (60000.0 / deltaTime);  // convert to RPM (revolutions per minute)

        RPM = revs;

        // Update for next calculation
        lastPulseCount = pulseCount;
        lastCalcTime = currentTime;
    }
}


