//
// Created by Øystein Bringsli.
//

#ifndef MOTORDRIVER4416_PRINTINFO_HPP
#define MOTORDRIVER4416_PRINTINFO_HPP

#include <Arduino.h>

class PrintInfo {
public:

    static constexpr int encoderTicks = 13;
    constexpr static double gearing = 30613.0 / 1500.0;

    static float RPM;
    static void printRPM(unsigned long int desiredPosition);
    static void calculateRPM();
};


#endif //MOTORDRIVER4416_PRINTINFO_HPP
