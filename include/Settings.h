//
// Persistent user settings stored in NVS.
//

#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <Preferences.h>

#include "AppConfig.h"

class Settings {
public:
    void begin();
    void save();

    // Calibration. extensionSign is the sign of the encoder delta when the
    // string is pulled OUT (the direction the bar travels when lowering).
    float countsPerMeter = AppConfig::DEFAULT_COUNTS_PER_METER;
    int8_t extensionSign = 1;
    bool calibrated = false;

    // String take-up motor.
    bool takeupEnabled = true;
    uint8_t takeupPercent = AppConfig::TAKEUP_DEFAULT_PERCENT;
    bool takeupDir = false;

    // Rep detection.
    float minRepRomCm = AppConfig::DEFAULT_MIN_REP_ROM_CM;
    float velocityDeadband = AppConfig::DEFAULT_VELOCITY_DEADBAND;

    // Last used values, restored in the web UI.
    float lastWeightKg = 60.0f;
    String lastExercise = "Squat";

    uint8_t takeupDuty() const;

private:
    Preferences prefs;
};

#endif // SETTINGS_H
