//
// Persistent user settings stored in NVS.
//

#include "Settings.h"

void Settings::begin() {
    prefs.begin("barbell", false);

    countsPerMeter = prefs.getFloat("cpm", AppConfig::DEFAULT_COUNTS_PER_METER);
    extensionSign = static_cast<int8_t>(prefs.getChar("extsign", 1));
    calibrated = prefs.getBool("calibrated", false);

    takeupEnabled = prefs.getBool("takeup_en", true);
    takeupPercent = prefs.getUChar("takeup_pct", AppConfig::TAKEUP_DEFAULT_PERCENT);
    takeupDir = prefs.getBool("takeup_dir", false);

    minRepRomCm = prefs.getFloat("min_rom", AppConfig::DEFAULT_MIN_REP_ROM_CM);
    velocityDeadband = prefs.getFloat("deadband", AppConfig::DEFAULT_VELOCITY_DEADBAND);

    lastWeightKg = prefs.getFloat("last_weight", 60.0f);
    lastExercise = prefs.getString("last_ex", "Squat");

    if (countsPerMeter < 10.0f) {
        countsPerMeter = AppConfig::DEFAULT_COUNTS_PER_METER;
        calibrated = false;
    }
    if (extensionSign == 0) {
        extensionSign = 1;
    }
    if (takeupPercent > 100) {
        takeupPercent = 100;
    }
    if (minRepRomCm < 1.0f) {
        minRepRomCm = AppConfig::DEFAULT_MIN_REP_ROM_CM;
    }
    if (velocityDeadband < 0.0f || velocityDeadband > 0.5f) {
        velocityDeadband = AppConfig::DEFAULT_VELOCITY_DEADBAND;
    }
}

void Settings::save() {
    prefs.putFloat("cpm", countsPerMeter);
    prefs.putChar("extsign", extensionSign);
    prefs.putBool("calibrated", calibrated);

    prefs.putBool("takeup_en", takeupEnabled);
    prefs.putUChar("takeup_pct", takeupPercent);
    prefs.putBool("takeup_dir", takeupDir);

    prefs.putFloat("min_rom", minRepRomCm);
    prefs.putFloat("deadband", velocityDeadband);

    prefs.putFloat("last_weight", lastWeightKg);
    prefs.putString("last_ex", lastExercise);
}

uint8_t Settings::takeupDuty() const {
    const uint32_t duty = static_cast<uint32_t>(takeupPercent) * AppConfig::TAKEUP_MAX_DUTY / 100U;
    return static_cast<uint8_t>(duty > 255 ? 255 : duty);
}
