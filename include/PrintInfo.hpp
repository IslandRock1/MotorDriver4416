//
// Created by Øystein Bringsli.
//

#ifndef MOTORDRIVER4416_PRINTINFO_HPP
#define MOTORDRIVER4416_PRINTINFO_HPP

#include <Arduino.h>

class PrintInfo {
public:
    PrintInfo();

    static constexpr int encoderTicks = 13;
    constexpr static double gearing = 30613.0 / 1500.0;

    static float RPM;
    static volatile bool motordir;
    static volatile uint32_t pulseCount;
    static void printRPM();
    static void calculateRPM();
};


#endif //MOTORDRIVER4416_PRINTINFO_HPP
