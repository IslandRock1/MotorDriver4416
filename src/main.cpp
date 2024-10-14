
#include <Arduino.h>
#include "PrintInfo.h"
#include "MotorController.h"

/*#include <WiFi.h>
#include <ArduinoOTA.h>

const char* ssid = "edge 40 neo_6693";
const char* password = "cjsxdinb8a5hk6y";*/

auto lastPrint = millis();

auto initTime = millis();
bool hasInitialized = false;

auto lastMotorTime = millis();
bool lastMotorDir = false;

MotorController motorController;



void setup() {
    Serial.begin(115200);
    Serial.println("Serial configured.");

    motorController.initInterrupt();
    initTime = millis();
}

void loop() {

    if (!hasInitialized && (initTime + 5000 < millis())) {
        Serial.println("Init start position.");

        initTime = millis();

        motorController.incrementPositionRounds(1.0);
    }

    if (lastPrint + 50 < millis()) {
        PrintInfo::printRPM(motorController.desiredPosition);
        lastPrint = millis();
    }

    motorController.updatePosition();
}