//
// Barbell power meter / string encoder firmware.
//
// The encoder measures string travel. Combined with the weight entered in the
// web UI this gives bar velocity and concentric power (watts). The motor only
// applies a light take-up torque so the string spools back in.
//

#include <Arduino.h>

#include "MotorController.h"
#include "PowerMeter.h"
#include "SessionRecorder.h"
#include "Settings.h"
#include "WebApp.h"

MotorController motorController;
Settings settings;
SessionRecorder recorder;
PowerMeter powerMeter(motorController, settings, recorder);
WebApp webApp(powerMeter, settings, recorder);

void setup() {
    Serial.begin(115200);
    delay(250);
    Serial.println();
    Serial.println("=== Barbell Power Meter ===");

    motorController.initInterrupt();
    settings.begin();
    recorder.begin();
    powerMeter.begin();
    webApp.begin();

    if (!settings.calibrated) {
        Serial.println("[setup] Not calibrated. Open the web UI -> Settings and do the 1 m pull.");
    }
    Serial.printf("[setup] Take-up %s at %u%% (direction %s), deadband %.2f m/s, min ROM %.0f cm\n",
                  settings.takeupEnabled ? "enabled" : "disabled",
                  static_cast<unsigned>(settings.takeupPercent),
                  settings.takeupDir ? "B" : "A",
                  settings.velocityDeadband, settings.minRepRomCm);
    Serial.println("[setup] Connect your phone to WiFi \"BarbellPower\" and open http://192.168.4.1");
}

void loop() {
    webApp.handle();
    powerMeter.update(millis());
    delay(1);
}
