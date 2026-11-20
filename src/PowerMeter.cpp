//
// Converts encoder travel into bar velocity, power and rep statistics.
//

#include "PowerMeter.h"

#include <math.h>
#include <stdio.h>

namespace {

float clampFloat(float value, float low, float high) {
    if (isnan(value) || isinf(value)) {
        return low;
    }
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

String jsonEscape(const String &input) {
    String out;
    out.reserve(input.length() + 8);
    for (size_t i = 0; i < input.length(); ++i) {
        const char c = input[i];
        switch (c) {
            case '"':
                out += "\\\"";
                break;
            case '\\':
                out += "\\\\";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\r':
                out += "\\r";
                break;
            case '\t':
                out += "\\t";
                break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    out += ' ';
                } else {
                    out += c;
                }
                break;
        }
    }
    return out;
}

String csvEscape(const String &input) {
    bool needsQuotes = false;
    for (size_t i = 0; i < input.length(); ++i) {
        const char c = input[i];
        if (c == ',' || c == '"' || c == '\n' || c == '\r') {
            needsQuotes = true;
            break;
        }
    }
    if (!needsQuotes) {
        return input;
    }

    String out = "\"";
    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] == '"') {
            out += "\"\"";
        } else {
            out += input[i];
        }
    }
    out += "\"";
    return out;
}

} // namespace

PowerMeter::PowerMeter(MotorController &motorRef, Settings &settingsRef, SessionRecorder &recorderRef)
    : motor(motorRef), settings(settingsRef), recorder(recorderRef) {
}

void PowerMeter::begin() {
    manualZeroRaw = motor.getPosition();
    lastRaw = manualZeroRaw;
    lastSampleUs = micros();
    previousSampleUs = lastSampleUs;
    resetLiveStats();
    resetRepState(0.0f);
}

float PowerMeter::metersPerCount() const {
    if (settings.countsPerMeter < 1.0f) {
        return 1.0f / AppConfig::DEFAULT_COUNTS_PER_METER;
    }
    return 1.0f / settings.countsPerMeter;
}

float PowerMeter::liftSign() const {
    return settings.extensionSign >= 0 ? -1.0f : 1.0f;
}

float PowerMeter::positionFromRaw(long long raw) const {
    const long long zero = active ? sessionZeroRaw : manualZeroRaw;
    return static_cast<float>(raw - zero) * metersPerCount() * liftSign();
}

void PowerMeter::update(uint32_t nowMs) {
    applyTakeup();

    const uint32_t nowUs = micros();
    if (lastSampleUs == 0) {
        lastSampleUs = nowUs;
        return;
    }
    if (static_cast<uint32_t>(nowUs - lastSampleUs) < AppConfig::SAMPLE_INTERVAL_US) {
        return;
    }

    lastSampleUs = nowUs;
    sample(nowMs);

    if (active && (nowMs - sessionStartMs) > AppConfig::MAX_SESSION_MS) {
        stopSession();
    }
}

void PowerMeter::sample(uint32_t nowMs) {
    const uint32_t nowUs = micros();
    float dt = (nowUs - previousSampleUs) / 1000000.0f;
    previousSampleUs = nowUs;
    if (dt <= 0.0001f || dt > 0.5f) {
        dt = AppConfig::SAMPLE_INTERVAL_US / 1000000.0f;
    }

    lastRaw = motor.getPosition();
    const float newPosition = positionFromRaw(lastRaw);
    const float rawVelocity = (newPosition - prevPositionM) / dt;
    prevPositionM = newPosition;

    const float alpha = haveVelocity ? 0.45f : 1.0f;
    velocity = alpha * rawVelocity + (1.0f - alpha) * velocity;
    haveVelocity = true;
    positionM = newPosition;

    if (!active) {
        return;
    }

    float power = 0.0f;
    if (velocity > 0.0f) {
        power = weightKg * AppConfig::GRAVITY * velocity;
    }

    updateRepDetection(newPosition, nowMs, dt, power);

    if (nowMs - lastLogMs >= AppConfig::LOG_INTERVAL_MS) {
        recorder.logSample(nowMs - sessionStartMs, newPosition, velocity, power, phase, repCount);
        lastLogMs = nowMs;
    }
}

void PowerMeter::updateRepDetection(float pos, uint32_t nowMs, float dt, float power) {
    const float deadband = settings.velocityDeadband;
    const float minRomM = settings.minRepRomCm / 100.0f;
    const bool movingUp = velocity > deadband;
    const bool movingDown = velocity < -deadband;

    if (inAscent) {
        if (movingUp) {
            if (pos > repTopPos) {
                repTopPos = pos;
            }
            lastMotionMs = nowMs;
            if (velocity > repPeakVelocity) {
                repPeakVelocity = velocity;
            }
            if (power > repPeakPower) {
                repPeakPower = power;
            }
            repSumPower += power;
            repPowerSamples++;
            repWorkJ += power * dt;
        }

        if (movingDown && (repTopPos - pos) > 0.02f) {
            // Reversal at the top: finish the rep and start the eccentric phase.
            const float top = repTopPos;
            finishRep(nowMs);
            inAscent = false;
            inDescent = true;
            repTopPos = top;
            repBottomPos = pos;
            lastMotionMs = nowMs;
            phase = PHASE_DESCENT;
            return;
        }

        if (nowMs - lastMotionMs > 300) {
            finishRep(nowMs);
            inAscent = false;
            inDescent = false;
            idleRefPos = pos;
            repTopPos = pos;
            phase = PHASE_IDLE;
            return;
        }

        phase = PHASE_ASCENT;
        return;
    }

    if (inDescent) {
        if (pos < repBottomPos) {
            repBottomPos = pos;
        }
        if (movingUp || movingDown) {
            lastMotionMs = nowMs;
        }

        if (movingUp && (pos - repBottomPos) > 0.02f) {
            inDescent = false;
            inAscent = true;
            resetRepAccumulators();
            ascentStartMs = nowMs;
            lastMotionMs = nowMs;
            repTopPos = pos;
            repPeakVelocity = velocity;
            phase = PHASE_ASCENT;
            return;
        }

        if (nowMs - lastMotionMs > 2000) {
            // The lifter paused before starting the concentric phase.
            inDescent = false;
            idleRefPos = pos;
            repTopPos = pos;
            phase = PHASE_IDLE;
            return;
        }

        phase = PHASE_DESCENT;
        return;
    }

    // Idle: wait for enough movement in either direction.
    if (movingDown && (idleRefPos - pos) > (minRomM * 0.5f)) {
        inDescent = true;
        inAscent = false;
        repTopPos = idleRefPos;
        repBottomPos = pos;
        ascentStartMs = nowMs;
        lastMotionMs = nowMs;
        phase = PHASE_DESCENT;
        return;
    }

    if (movingUp && (pos - idleRefPos) > (minRomM * 0.5f)) {
        // The set started at the bottom, or the descent was too small to see.
        inAscent = true;
        inDescent = false;
        resetRepAccumulators();
        repBottomPos = idleRefPos;
        repTopPos = pos;
        ascentStartMs = nowMs;
        lastMotionMs = nowMs;
        repPeakVelocity = velocity;
        phase = PHASE_ASCENT;
        return;
    }

    if (fabs(velocity) < deadband) {
        idleRefPos = pos;
    }
    phase = PHASE_IDLE;
}

void PowerMeter::finishRep(uint32_t nowMs) {
    const float rom = repTopPos - repBottomPos;
    uint32_t motionEnd = lastMotionMs;
    if (motionEnd < ascentStartMs) {
        motionEnd = nowMs;
    }
    uint32_t duration = motionEnd > ascentStartMs ? (motionEnd - ascentStartMs) : 1;
    if (duration == 0) {
        duration = 1;
    }

    if (rom >= (settings.minRepRomCm / 100.0f)) {
        addRep(duration, rom);
    }
    resetRepAccumulators();
}

void PowerMeter::addRep(uint32_t durationMs, float romM) {
    if (repCount >= AppConfig::MAX_REPS) {
        return;
    }

    const float seconds = durationMs / 1000.0f;
    RepInfo rep;
    rep.index = repCount + 1;
    rep.romCm = romM * 100.0f;
    rep.durationMs = durationMs;
    rep.avgVelocity = seconds > 0.0f ? (romM / seconds) : 0.0f;
    rep.peakVelocity = repPeakVelocity;
    rep.avgPower = seconds > 0.0f ? (repWorkJ / seconds) : 0.0f;
    rep.peakPower = repPeakPower;
    rep.workJ = repWorkJ;

    if (repCount == 0) {
        maxTopPos = repTopPos;
        minBottomPos = repBottomPos;
    } else {
        if (repTopPos > maxTopPos) {
            maxTopPos = repTopPos;
        }
        if (repBottomPos < minBottomPos) {
            minBottomPos = repBottomPos;
        }
    }

    reps[repCount] = rep;
    repCount++;

    if (rep.peakPower > setPeakPower) {
        setPeakPower = rep.peakPower;
    }
    if (rep.peakVelocity > setPeakVelocity) {
        setPeakVelocity = rep.peakVelocity;
    }
    setWorkJ += rep.workJ;
    setTutMs += rep.durationMs;
    setConcentricDistance += romM;
}

void PowerMeter::resetRepAccumulators() {
    repPeakVelocity = 0.0f;
    repPeakPower = 0.0f;
    repSumPower = 0.0f;
    repPowerSamples = 0;
    repWorkJ = 0.0f;
}

void PowerMeter::resetRepState(float pos) {
    phase = PHASE_IDLE;
    inDescent = false;
    inAscent = false;
    idleRefPos = pos;
    repTopPos = pos;
    repBottomPos = pos;
    ascentStartMs = millis();
    lastMotionMs = millis();
    resetRepAccumulators();
}

void PowerMeter::resetLiveStats() {
    repCount = 0;
    setPeakPower = 0.0f;
    setPeakVelocity = 0.0f;
    setWorkJ = 0.0f;
    setTutMs = 0;
    setConcentricDistance = 0.0f;
    maxTopPos = -1000.0f;
    minBottomPos = 1000.0f;
    resetRepAccumulators();
}

bool PowerMeter::startSession(float weight, const String &exerciseName, uint32_t epoch, String &error) {
    if (active) {
        error = "A set is already running.";
        return false;
    }
    if (calibrating) {
        error = "Finish or cancel calibration first.";
        return false;
    }
    if (weight < 0.5f || weight > 1000.0f) {
        error = "Weight must be between 0.5 and 1000 kg.";
        return false;
    }

    weightKg = weight;
    exercise = exerciseName;
    exercise.trim();
    if (exercise.length() == 0) {
        exercise = "Custom";
    }
    if (exercise.length() > 40) {
        exercise = exercise.substring(0, 40);
    }

    sessionEpoch = epoch;
    sessionStartMs = millis();
    sessionCalibrated = settings.calibrated;
    sessionZeroRaw = motor.getPosition();
    manualZeroRaw = sessionZeroRaw;
    lastRaw = sessionZeroRaw;
    positionM = 0.0f;
    prevPositionM = 0.0f;
    velocity = 0.0f;
    haveVelocity = false;
    lastLogMs = sessionStartMs;
    lastSampleUs = micros();
    previousSampleUs = lastSampleUs;

    resetLiveStats();
    resetRepState(0.0f);

    sessionId = "live";
    recording = recorder.start(weightKg, exercise, epoch, sessionStartMs);
    if (recording) {
        sessionId = recorder.currentId();
        recorder.writeMeta(buildMetadata(false, sessionStartMs));
    } else {
        Serial.println("[session] Recording unavailable; running live only.");
    }

    active = true;
    lastSummary = Summary();

    settings.lastWeightKg = weightKg;
    settings.lastExercise = exercise;
    settings.save();

    Serial.printf("[session] Started %s: %.1f kg, %s\n", sessionId.c_str(), weightKg, exercise.c_str());
    return true;
}

bool PowerMeter::stopSession() {
    if (!active) {
        return false;
    }

    const uint32_t now = millis();
    if (inAscent) {
        finishRep(now);
        inAscent = false;
    }

    const uint32_t durationMs = now > sessionStartMs ? (now - sessionStartMs) : 0;
    const float tutSeconds = setTutMs / 1000.0f;

    lastSummary = Summary();
    lastSummary.valid = true;
    lastSummary.id = sessionId;
    lastSummary.epoch = sessionEpoch;
    lastSummary.exercise = exercise;
    lastSummary.weightKg = weightKg;
    lastSummary.reps = repCount;
    lastSummary.durationMs = durationMs;
    lastSummary.avgPower = tutSeconds > 0.0f ? (setWorkJ / tutSeconds) : 0.0f;
    lastSummary.peakPower = setPeakPower;
    lastSummary.avgVelocity = tutSeconds > 0.0f ? (setConcentricDistance / tutSeconds) : 0.0f;
    lastSummary.peakVelocity = setPeakVelocity;
    lastSummary.romCm = repCount > 0 ? ((maxTopPos - minBottomPos) * 100.0f) : 0.0f;
    lastSummary.workJ = setWorkJ;
    lastSummary.tutMs = setTutMs;
    lastSummary.calibrated = sessionCalibrated;

    if (recording) {
        recorder.flush();
        recorder.finish();
        recorder.writeMeta(buildMetadata(true, now));
        recorder.writeSummaryLine(buildSummaryCsvLine(now));
        recording = false;
    }

    active = false;
    phase = PHASE_IDLE;
    inDescent = false;
    inAscent = false;

    Serial.printf("[session] Stopped: %u reps, avg %.1f W, peak %.1f W\n",
                  static_cast<unsigned>(repCount), lastSummary.avgPower, lastSummary.peakPower);
    return true;
}

PowerMeter::Snapshot PowerMeter::snapshot(uint32_t nowMs) {
    Snapshot snap;
    const long long raw = motor.getPosition();

    snap.active = active;
    snap.calibrating = calibrating;
    snap.recording = active && recording;
    snap.id = sessionId;
    snap.exercise = exercise;
    snap.weightKg = weightKg;
    snap.elapsedMs = active && nowMs > sessionStartMs ? (nowMs - sessionStartMs) : 0;
    snap.reps = repCount;
    snap.phase = active ? phase : PHASE_IDLE;
    snap.positionM = positionM;
    snap.velocity = velocity;
    snap.power = (active && velocity > 0.0f) ? (weightKg * AppConfig::GRAVITY * velocity) : 0.0f;
    snap.peakPower = setPeakPower;
    if (inAscent && repPeakPower > snap.peakPower) {
        snap.peakPower = repPeakPower;
    }
    snap.workJ = setWorkJ + (inAscent ? repWorkJ : 0.0f);
    snap.tutMs = setTutMs;
    if (inAscent && nowMs > ascentStartMs) {
        snap.tutMs += (nowMs - ascentStartMs);
    }
    const float tutSeconds = snap.tutMs / 1000.0f;
    snap.avgPower = tutSeconds > 0.0f ? (snap.workJ / tutSeconds) : 0.0f;
    if (repCount > 0) {
        snap.romCm = (maxTopPos - minBottomPos) * 100.0f;
    } else if (inAscent) {
        snap.romCm = (repTopPos - repBottomPos) * 100.0f;
    } else {
        snap.romCm = 0.0f;
    }

    snap.rawPosition = raw;
    snap.calibrationCounts = calibrating ? (raw - calibrationStartRaw) : 0;
    snap.calibrated = settings.calibrated;
    snap.countsPerMeter = settings.countsPerMeter;
    snap.extensionSign = settings.extensionSign;
    snap.takeupEnabled = settings.takeupEnabled;
    snap.takeupPercent = settings.takeupPercent;
    snap.takeupDir = settings.takeupDir;
    snap.minRepRomCm = settings.minRepRomCm;
    snap.velocityDeadband = settings.velocityDeadband;

    const uint16_t count = repCount < AppConfig::MAX_STATUS_REPS ? repCount : AppConfig::MAX_STATUS_REPS;
    snap.lastRepCount = static_cast<uint8_t>(count);
    for (uint16_t i = 0; i < count; ++i) {
        snap.lastReps[i] = reps[repCount - count + i];
    }

    snap.lastSummary = lastSummary;
    return snap;
}

bool PowerMeter::startCalibration(String &error) {
    if (active) {
        error = "Stop the current set before calibrating.";
        return false;
    }
    if (calibrating) {
        error = "Calibration is already running.";
        return false;
    }
    calibrationStartRaw = motor.getPosition();
    lastRaw = calibrationStartRaw;
    calibrating = true;
    Serial.println("[calibration] Started. Pull the string out exactly 1.00 m.");
    return true;
}

bool PowerMeter::finishCalibration(String &message) {
    if (!calibrating) {
        message = "Calibration is not running.";
        return false;
    }

    const long long delta = motor.getPosition() - calibrationStartRaw;
    const long long magnitude = delta >= 0 ? delta : -delta;
    if (magnitude < 100) {
        message = "Not enough movement. Pull the string out 1.00 m, then press Finish.";
        return false;
    }

    settings.countsPerMeter = static_cast<float>(magnitude);
    settings.extensionSign = delta >= 0 ? 1 : -1;
    settings.calibrated = true;
    settings.save();

    calibrating = false;
    zeroPosition();

    char buffer[128];
    snprintf(buffer, sizeof(buffer),
             "Calibrated: %.1f counts per metre. Lift direction was set from the pull.",
             settings.countsPerMeter);
    message = buffer;
    Serial.printf("[calibration] %s\n", buffer);
    return true;
}

void PowerMeter::cancelCalibration() {
    calibrating = false;
    Serial.println("[calibration] Cancelled.");
}

bool PowerMeter::autoDetectTakeupDirection(String &message, bool &newDir) {
    if (active) {
        message = "Stop the current set first.";
        return false;
    }
    if (!settings.calibrated) {
        message = "Calibrate the 1 m string travel first.";
        return false;
    }
    if (settings.takeupPercent == 0) {
        message = "Set the take-up strength above 0 % first.";
        return false;
    }

    uint8_t duty = settings.takeupDuty();
    if (duty < 20) {
        duty = 20;
    }

    const bool originalDir = settings.takeupDir;
    const long long start = motor.getPosition();

    Serial.printf("[takeup] Testing direction %s at duty %u\n", settings.takeupDir ? "B" : "A",
                  static_cast<unsigned>(duty));
    motor.controlMotorSpeed(duty, settings.takeupDir);
    delay(500);
    motor.stopMotor();
    lastMotorDuty = -1;

    const long long delta = motor.getPosition() - start;
    const long long magnitude = delta >= 0 ? delta : -delta;
    if (magnitude < 20) {
        settings.takeupDir = originalDir;
        message = "No movement detected. Increase the take-up strength and try again.";
        return false;
    }

    const int deltaSign = delta > 0 ? 1 : -1;
    const int retractSign = settings.extensionSign >= 0 ? -1 : 1;
    if (deltaSign != retractSign) {
        settings.takeupDir = !settings.takeupDir;
    }
    settings.save();
    newDir = settings.takeupDir;
    zeroPosition();

    message = "Take-up direction detected and saved.";
    Serial.printf("[takeup] Detected direction %s\n", settings.takeupDir ? "B" : "A");
    return true;
}

void PowerMeter::zeroPosition() {
    manualZeroRaw = motor.getPosition();
    if (active) {
        sessionZeroRaw = manualZeroRaw;
        resetLiveStats();
    }
    lastRaw = manualZeroRaw;
    positionM = 0.0f;
    prevPositionM = 0.0f;
    velocity = 0.0f;
    haveVelocity = false;
    resetRepState(0.0f);
}

void PowerMeter::setTakeupEnabled(bool enabled) {
    settings.takeupEnabled = enabled;
    settings.save();
}

void PowerMeter::setTakeupStrength(uint8_t percent) {
    if (percent > 100) {
        percent = 100;
    }
    settings.takeupPercent = percent;
    settings.save();
}

void PowerMeter::setTakeupDir(bool dir) {
    settings.takeupDir = dir;
    settings.save();
}

void PowerMeter::flipLiftDirection() {
    settings.extensionSign = settings.extensionSign >= 0 ? -1 : 1;
    settings.save();
    zeroPosition();
}

void PowerMeter::applyTakeup() {
    if (calibrating) {
        setMotor(0, settings.takeupDir);
        return;
    }
    if (!settings.takeupEnabled || settings.takeupPercent == 0) {
        setMotor(0, settings.takeupDir);
        return;
    }
    setMotor(settings.takeupDuty(), settings.takeupDir);
}

void PowerMeter::setMotor(int duty, bool dir) {
    if (duty == lastMotorDuty && dir == lastMotorDir) {
        return;
    }
    if (duty <= 0) {
        motor.stopMotor();
    } else {
        motor.controlMotorSpeed(duty, dir);
    }
    lastMotorDuty = duty;
    lastMotorDir = dir;
}

String PowerMeter::buildMetadata(bool finished, uint32_t nowMs) const {
    const uint32_t durationMs = nowMs > sessionStartMs ? (nowMs - sessionStartMs) : 0;
    const float tutSeconds = setTutMs / 1000.0f;
    const float avgPower = tutSeconds > 0.0f ? (setWorkJ / tutSeconds) : 0.0f;
    const float avgVelocity = tutSeconds > 0.0f ? (setConcentricDistance / tutSeconds) : 0.0f;
    const float romCm = repCount > 0 ? ((maxTopPos - minBottomPos) * 100.0f) : 0.0f;

    String out;
    out.reserve(1600);
    out += "{";
    out += "\"id\":\"" + jsonEscape(sessionId) + "\",";
    out += "\"epoch\":" + String(sessionEpoch) + ",";
    out += "\"exercise\":\"" + jsonEscape(exercise) + "\",";
    out += "\"weightKg\":" + String(clampFloat(weightKg, 0.0f, 1000.0f), 2) + ",";
    out += "\"finished\":";
    out += finished ? "true" : "false";
    out += ",";
    out += "\"durationS\":" + String(durationMs / 1000.0f, 2) + ",";
    out += "\"reps\":" + String(repCount) + ",";
    out += "\"avgPowerW\":" + String(clampFloat(avgPower, 0.0f, 100000.0f), 1) + ",";
    out += "\"peakPowerW\":" + String(clampFloat(setPeakPower, 0.0f, 100000.0f), 1) + ",";
    out += "\"avgVelocityMps\":" + String(clampFloat(avgVelocity, 0.0f, 20.0f), 3) + ",";
    out += "\"peakVelocityMps\":" + String(clampFloat(setPeakVelocity, 0.0f, 20.0f), 3) + ",";
    out += "\"romCm\":" + String(clampFloat(romCm, 0.0f, 1000.0f), 1) + ",";
    out += "\"workJ\":" + String(clampFloat(setWorkJ, 0.0f, 1000000.0f), 1) + ",";
    out += "\"tutS\":" + String(setTutMs / 1000.0f, 2) + ",";
    out += "\"calibrated\":";
    out += sessionCalibrated ? "true" : "false";
    out += ",";
    out += "\"countsPerMeter\":" + String(settings.countsPerMeter, 1) + ",";
    out += "\"repList\":[";

    for (uint16_t i = 0; i < repCount; ++i) {
        if (i > 0) {
            out += ",";
        }
        const RepInfo &r = reps[i];
        out += "{";
        out += "\"i\":" + String(r.index) + ",";
        out += "\"romCm\":" + String(r.romCm, 1) + ",";
        out += "\"ms\":" + String(r.durationMs) + ",";
        out += "\"avgV\":" + String(r.avgVelocity, 3) + ",";
        out += "\"peakV\":" + String(r.peakVelocity, 3) + ",";
        out += "\"avgW\":" + String(r.avgPower, 1) + ",";
        out += "\"peakW\":" + String(r.peakPower, 1) + ",";
        out += "\"workJ\":" + String(r.workJ, 1);
        out += "}";
    }

    out += "]}";
    return out;
}

String PowerMeter::buildSummaryCsvLine(uint32_t nowMs) const {
    const uint32_t durationMs = nowMs > sessionStartMs ? (nowMs - sessionStartMs) : 0;
    const float tutSeconds = setTutMs / 1000.0f;
    const float avgPower = tutSeconds > 0.0f ? (setWorkJ / tutSeconds) : 0.0f;
    const float avgVelocity = tutSeconds > 0.0f ? (setConcentricDistance / tutSeconds) : 0.0f;
    const float romCm = repCount > 0 ? ((maxTopPos - minBottomPos) * 100.0f) : 0.0f;

    String line;
    line.reserve(160);
    line += sessionId;
    line += ",";
    line += String(sessionEpoch);
    line += ",";
    line += csvEscape(exercise);
    line += ",";
    line += String(weightKg, 2);
    line += ",";
    line += String(repCount);
    line += ",";
    line += String(durationMs / 1000.0f, 2);
    line += ",";
    line += String(avgPower, 1);
    line += ",";
    line += String(setPeakPower, 1);
    line += ",";
    line += String(avgVelocity, 3);
    line += ",";
    line += String(setPeakVelocity, 3);
    line += ",";
    line += String(romCm, 1);
    line += ",";
    line += String(setWorkJ, 1);
    line += ",";
    line += String(tutSeconds, 2);
    return line;
}
