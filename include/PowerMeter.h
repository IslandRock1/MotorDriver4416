//
// Converts encoder travel into bar velocity, power and rep statistics.
//

#ifndef POWERMETER_H
#define POWERMETER_H

#include <Arduino.h>

#include "AppConfig.h"
#include "MotorController.h"
#include "SessionRecorder.h"
#include "Settings.h"

class PowerMeter {
public:
    enum Phase : uint8_t {
        PHASE_IDLE = 0,
        PHASE_DESCENT = 1,
        PHASE_ASCENT = 2
    };

    struct RepInfo {
        uint16_t index = 0;
        float romCm = 0;
        uint32_t durationMs = 0;
        float avgVelocity = 0;
        float peakVelocity = 0;
        float avgPower = 0;
        float peakPower = 0;
        float workJ = 0;
    };

    struct Summary {
        bool valid = false;
        String id;
        uint32_t epoch = 0;
        String exercise;
        float weightKg = 0;
        uint16_t reps = 0;
        uint32_t durationMs = 0;
        float avgPower = 0;
        float peakPower = 0;
        float avgVelocity = 0;
        float peakVelocity = 0;
        float romCm = 0;
        float workJ = 0;
        uint32_t tutMs = 0;
        bool calibrated = false;
    };

    struct Snapshot {
        bool active = false;
        bool calibrating = false;
        bool recording = false;
        String id;
        String exercise;
        float weightKg = 0;
        uint32_t elapsedMs = 0;
        uint16_t reps = 0;
        uint8_t phase = PHASE_IDLE;
        float positionM = 0;
        float velocity = 0;
        float power = 0;
        float peakPower = 0;
        float avgPower = 0;
        float romCm = 0;
        float workJ = 0;
        uint32_t tutMs = 0;
        long long rawPosition = 0;
        long long calibrationCounts = 0;

        bool calibrated = false;
        float countsPerMeter = 0;
        int8_t extensionSign = 1;
        bool takeupEnabled = false;
        uint8_t takeupPercent = 0;
        bool takeupDir = false;
        float minRepRomCm = 0;
        float velocityDeadband = 0;

        uint8_t lastRepCount = 0;
        RepInfo lastReps[AppConfig::MAX_STATUS_REPS];
        Summary lastSummary;
    };

    PowerMeter(MotorController &motor, Settings &settings, SessionRecorder &recorder);

    void begin();
    void update(uint32_t nowMs);
    Snapshot snapshot(uint32_t nowMs);

    bool startSession(float weightKg, const String &exercise, uint32_t epoch, String &error);
    bool stopSession();
    bool sessionActive() const { return active; }

    bool startCalibration(String &error);
    bool finishCalibration(String &message);
    void cancelCalibration();
    bool calibrationActive() const { return calibrating; }

    bool autoDetectTakeupDirection(String &message, bool &newDir);
    void zeroPosition();

    void setTakeupEnabled(bool enabled);
    void setTakeupStrength(uint8_t percent);
    void setTakeupDir(bool dir);
    void flipLiftDirection();

private:
    void sample(uint32_t nowMs);
    void updateRepDetection(float pos, uint32_t nowMs, float dt, float power);
    void finishRep(uint32_t nowMs);
    void addRep(uint32_t durationMs, float romM);
    void resetLiveStats();
    void resetRepState(float pos);
    void resetRepAccumulators();
    void applyTakeup();
    void setMotor(int duty, bool dir);

    float metersPerCount() const;
    float liftSign() const;
    float positionFromRaw(long long raw) const;

    String buildMetadata(bool finished, uint32_t nowMs) const;
    String buildSummaryCsvLine(uint32_t nowMs) const;

    MotorController &motor;
    Settings &settings;
    SessionRecorder &recorder;

    bool active = false;
    bool calibrating = false;
    bool haveVelocity = false;
    bool recording = false;

    String sessionId;
    String exercise;
    float weightKg = 0;
    uint32_t sessionStartMs = 0;
    uint32_t sessionEpoch = 0;
    bool sessionCalibrated = false;
    long long sessionZeroRaw = 0;
    long long manualZeroRaw = 0;
    long long lastRaw = 0;

    float positionM = 0;
    float prevPositionM = 0;
    float velocity = 0;
    uint32_t lastSampleUs = 0;
    uint32_t previousSampleUs = 0;
    uint32_t lastLogMs = 0;

    uint16_t repCount = 0;
    RepInfo reps[AppConfig::MAX_REPS];
    float setPeakPower = 0;
    float setPeakVelocity = 0;
    float setWorkJ = 0;
    uint32_t setTutMs = 0;
    float setConcentricDistance = 0;
    float maxTopPos = 0;
    float minBottomPos = 0;

    uint8_t phase = PHASE_IDLE;
    bool inDescent = false;
    bool inAscent = false;
    float idleRefPos = 0;
    float repTopPos = 0;
    float repBottomPos = 0;
    uint32_t ascentStartMs = 0;
    uint32_t lastMotionMs = 0;
    float repPeakVelocity = 0;
    float repPeakPower = 0;
    float repSumPower = 0;
    uint16_t repPowerSamples = 0;
    float repWorkJ = 0;

    long long calibrationStartRaw = 0;

    Summary lastSummary;

    int lastMotorDuty = -1;
    bool lastMotorDir = false;
};

#endif // POWERMETER_H
