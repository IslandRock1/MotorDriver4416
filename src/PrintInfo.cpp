//
// Created by Øystein Bringsli.
//

#include "PrintInfo.hpp"

PrintInfo::PrintInfo() {
    pulseCount = 0;
    motordir = false;
}

void printRPM() {

    PrintInfo::calculateRPM();

    Serial.print("Direction: ");
    if (PrintInfo::motordir) {
        Serial.print("CW @ ");
    } else {
        Serial.print("CCW @ ");
    }
    Serial.print((int)PrintInfo::RPM); Serial.println(" RPM");
}

void calculateRPM() {
    static uint32_t lastPulseCount = 0;
    static uint32_t lastCalcTime = 0;

    uint32_t currentTime = millis();
    uint32_t deltaTime = currentTime - lastCalcTime;

    if (deltaTime >= 1000) {  // Calculate every 1 second
        uint32_t deltaPulses = PrintInfo::pulseCount - lastPulseCount;

        // Calculate RPM
        float revs = deltaPulses;
        revs /= PrintInfo::gearing;                // account for gear ratio
        revs /= PrintInfo::encoderTicks;            // account for multiple ticks per rotation
        revs *= (60000.0 / deltaTime);  // convert to RPM (revolutions per minute)

        PrintInfo::RPM = revs;

        // Update for next calculation
        lastPulseCount = PrintInfo::pulseCount;
        lastCalcTime = currentTime;
    }
}


