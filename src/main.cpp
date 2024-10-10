
#include <Arduino.h>
#include "PrintInfo.h"
#include "MotorController.h"

MotorController motorController;

void setup() {
    Serial.begin(115200);
    Serial.println("Serial configured.");

    motorController.controlMotorSpeed(255, true);
}

auto lastPrint = millis();
void loop() {
    if (lastPrint + 50 < millis()) {
        PrintInfo::printRPM();
        lastPrint = millis();
    }
}