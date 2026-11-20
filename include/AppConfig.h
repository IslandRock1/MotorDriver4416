//
// Shared configuration for the barbell power meter firmware.
//

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <Arduino.h>

namespace AppConfig {

// WiFi access point created by the ESP32.
constexpr const char *AP_SSID = "BarbellPower";
constexpr const char *AP_PASSWORD = "barbell123";

// Physics / sampling.
constexpr float GRAVITY = 9.80665f;
constexpr uint32_t SAMPLE_INTERVAL_US = 20000;   // 50 Hz control/measurement loop
constexpr uint32_t LOG_INTERVAL_MS = 40;         // 25 Hz raw logging
constexpr uint32_t MAX_SESSION_MS = 30UL * 60UL * 1000UL;

// Calibration fallback. Only used until the user performs the 1 m calibration.
constexpr float DEFAULT_COUNTS_PER_METER = 1689.0f;

// Take-up motor. The UI exposes 0-100 %, mapped onto this safe duty ceiling.
constexpr uint8_t TAKEUP_MAX_DUTY = 60;
constexpr uint8_t TAKEUP_DEFAULT_PERCENT = 15;

// Rep detection defaults.
constexpr float DEFAULT_MIN_REP_ROM_CM = 10.0f;
constexpr float DEFAULT_VELOCITY_DEADBAND = 0.03f; // m/s

// Runtime limits.
constexpr uint16_t MAX_REPS = 250;
constexpr uint16_t MAX_STATUS_REPS = 12;
constexpr size_t MIN_FREE_BYTES = 120 * 1024;
constexpr size_t MAX_SESSION_BYTES = 1024 * 1024;
constexpr int MAX_SESSION_LIST = 60;

constexpr const char *SESSION_DIR = "/sessions";

} // namespace AppConfig

#endif // APP_CONFIG_H
