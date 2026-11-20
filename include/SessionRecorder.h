//
// Stores raw session samples and metadata on the ESP32's LittleFS partition.
//

#ifndef SESSIONRECORDER_H
#define SESSIONRECORDER_H

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>

#include "AppConfig.h"

class SessionRecorder {
public:
    bool begin();
    bool available() const;

    bool start(float weightKg, const String &exercise, uint32_t epoch, uint32_t nowMs);
    void logSample(uint32_t tMs, float posM, float velocity, float power, uint8_t phase, uint16_t rep);
    void flush();
    void finish();

    bool active() const { return isActive; }
    String currentId() const { return id; }

    bool writeMeta(const String &json);
    bool writeSummaryLine(const String &line);

    String listJson();
    String summaryCsv();
    bool remove(const String &sessionId);
    bool removeAll();

    size_t totalBytes() const;
    size_t usedBytes() const;
    size_t freeBytes() const;
    bool hasSpace();

    static bool validId(const String &sessionId);

private:
    bool ensureSpace();
    String nextId(uint32_t epoch);
    String metaPath(const String &sessionId) const;
    String csvPath(const String &sessionId) const;
    String sumPath(const String &sessionId) const;

    bool mounted = false;
    bool isActive = false;
    File logFile;
    String id;
    String buffer;
    uint32_t lastFlushMs = 0;
    size_t bytesWritten = 0;
};

#endif // SESSIONRECORDER_H
