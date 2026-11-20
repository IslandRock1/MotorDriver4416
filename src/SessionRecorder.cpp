//
// Stores raw session samples and metadata on the ESP32's LittleFS partition.
//

#include "SessionRecorder.h"

#include <algorithm>
#include <ctype.h>
#include <time.h>
#include <vector>

namespace {

const char *CSV_HEADER = "time_ms,position_m,velocity_mps,power_w,phase,rep\n";
const char *SUMMARY_HEADER =
    "id,epoch,exercise,weight_kg,reps,duration_s,avg_power_w,peak_power_w,"
    "avg_velocity_mps,peak_velocity_mps,rom_cm,work_j,tut_s\n";

String basenameOf(const char *fullName) {
    String name(fullName == nullptr ? "" : fullName);
    const int slash = name.lastIndexOf('/');
    if (slash >= 0) {
        name = name.substring(slash + 1);
    }
    return name;
}

void collectIds(std::vector<String> &ids, const char *extension) {
    if (!LittleFS.exists(AppConfig::SESSION_DIR)) {
        return;
    }

    File dir = LittleFS.open(AppConfig::SESSION_DIR);
    if (!dir) {
        return;
    }

    File entry = dir.openNextFile();
    while (entry) {
        if (!entry.isDirectory()) {
            String name = basenameOf(entry.name());
            if (name.endsWith(extension)) {
                const String base = name.substring(0, name.length() - strlen(extension));
                if (SessionRecorder::validId(base)) {
                    ids.push_back(base);
                }
            }
        }
        entry = dir.openNextFile();
    }
}

} // namespace

bool SessionRecorder::validId(const String &sessionId) {
    if (sessionId.length() == 0 || sessionId.length() > 40) {
        return false;
    }
    for (size_t i = 0; i < sessionId.length(); ++i) {
        const char c = sessionId[i];
        if (!isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-') {
            return false;
        }
    }
    return true;
}

bool SessionRecorder::begin() {
    mounted = LittleFS.begin(true);
    if (mounted) {
        LittleFS.mkdir(AppConfig::SESSION_DIR);
        Serial.printf("[storage] LittleFS mounted: %u / %u bytes used\n",
                      static_cast<unsigned>(usedBytes()), static_cast<unsigned>(totalBytes()));
    } else {
        Serial.println("[storage] LittleFS mount failed - sessions will not be saved.");
    }
    return mounted;
}

bool SessionRecorder::available() const {
    return mounted;
}

String SessionRecorder::csvPath(const String &sessionId) const {
    return String(AppConfig::SESSION_DIR) + "/" + sessionId + ".csv";
}

String SessionRecorder::metaPath(const String &sessionId) const {
    return String(AppConfig::SESSION_DIR) + "/" + sessionId + ".json";
}

String SessionRecorder::sumPath(const String &sessionId) const {
    return String(AppConfig::SESSION_DIR) + "/" + sessionId + ".sum.csv";
}

String SessionRecorder::nextId(uint32_t epoch) {
    String base;

    if (epoch > 1600000000UL) {
        time_t seconds = static_cast<time_t>(epoch);
        struct tm tmValue;
        gmtime_r(&seconds, &tmValue);
        char buffer[24];
        strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", &tmValue);
        base = buffer;
    } else {
        char buffer[40];
        snprintf(buffer, sizeof(buffer), "boot_%lu_%04x",
                 static_cast<unsigned long>(millis()), static_cast<unsigned>(esp_random() & 0xFFFF));
        base = buffer;
    }

    String candidate = base;
    uint16_t suffix = 1;
    while (LittleFS.exists(metaPath(candidate)) || LittleFS.exists(csvPath(candidate))) {
        candidate = base + "_" + String(suffix++);
    }
    return candidate;
}

bool SessionRecorder::ensureSpace() {
    if (!mounted) {
        return false;
    }

    uint16_t guard = 0;
    while (freeBytes() < AppConfig::MIN_FREE_BYTES && guard++ < 100) {
        std::vector<String> ids;
        collectIds(ids, ".json");
        if (ids.empty()) {
            break;
        }
        std::sort(ids.begin(), ids.end()); // oldest first
        bool removed = false;
        for (size_t i = 0; i < ids.size(); ++i) {
            if (isActive && ids[i] == id) {
                continue;
            }
            if (remove(ids[i])) {
                removed = true;
                break;
            }
        }
        if (!removed) {
            break;
        }
    }

    return freeBytes() >= AppConfig::MIN_FREE_BYTES;
}

bool SessionRecorder::start(float weightKg, const String &exercise, uint32_t epoch, uint32_t nowMs) {
    (void)weightKg;
    (void)exercise;

    if (!mounted || isActive) {
        return false;
    }
    if (!ensureSpace()) {
        Serial.println("[storage] Not enough free space for a new session.");
        return false;
    }

    id = nextId(epoch);
    logFile = LittleFS.open(csvPath(id), "w");
    if (!logFile) {
        Serial.println("[storage] Could not create session CSV.");
        return false;
    }

    const size_t written = logFile.print(CSV_HEADER);
    buffer = "";
    bytesWritten = written;
    lastFlushMs = nowMs;
    isActive = true;

    Serial.printf("[storage] Recording session %s\n", id.c_str());
    return true;
}

void SessionRecorder::logSample(uint32_t tMs, float posM, float velocity, float power, uint8_t phase,
                                uint16_t rep) {
    if (!isActive || !logFile) {
        return;
    }
    if (bytesWritten + buffer.length() > AppConfig::MAX_SESSION_BYTES) {
        return;
    }

    char line[96];
    snprintf(line, sizeof(line), "%lu,%.4f,%.3f,%.1f,%u,%u\n",
             static_cast<unsigned long>(tMs), posM, velocity, power,
             static_cast<unsigned>(phase), static_cast<unsigned>(rep));
    buffer += line;

    if (buffer.length() >= 1024 || millis() - lastFlushMs >= 1000) {
        flush();
    }
}

void SessionRecorder::flush() {
    if (!isActive || !logFile || buffer.length() == 0) {
        return;
    }
    const size_t written = logFile.print(buffer);
    bytesWritten += written;
    buffer = "";
    lastFlushMs = millis();
}

void SessionRecorder::finish() {
    if (!isActive) {
        return;
    }
    flush();
    if (logFile) {
        logFile.close();
    }
    isActive = false;
    Serial.printf("[storage] Closed session %s (%u bytes)\n", id.c_str(),
                  static_cast<unsigned>(bytesWritten));
}

bool SessionRecorder::writeMeta(const String &json) {
    if (!mounted || id.length() == 0) {
        return false;
    }
    File file = LittleFS.open(metaPath(id), "w");
    if (!file) {
        return false;
    }
    file.print(json);
    file.close();
    return true;
}

bool SessionRecorder::writeSummaryLine(const String &line) {
    if (!mounted || id.length() == 0) {
        return false;
    }
    File file = LittleFS.open(sumPath(id), "w");
    if (!file) {
        return false;
    }
    file.print(line);
    if (!line.endsWith("\n")) {
        file.print("\n");
    }
    file.close();
    return true;
}

String SessionRecorder::listJson() {
    String out;
    out.reserve(4096);
    out += "[";

    std::vector<String> ids;
    collectIds(ids, ".json");
    std::sort(ids.begin(), ids.end(), [](const String &a, const String &b) { return a > b; });

    bool first = true;
    const size_t limit = ids.size() < static_cast<size_t>(AppConfig::MAX_SESSION_LIST)
                             ? ids.size()
                             : static_cast<size_t>(AppConfig::MAX_SESSION_LIST);
    for (size_t i = 0; i < limit; ++i) {
        File file = LittleFS.open(metaPath(ids[i]), "r");
        if (!file) {
            continue;
        }
        String content = file.readString();
        file.close();
        if (content.length() < 2) {
            continue;
        }
        if (out.length() + content.length() > 30000) {
            break;
        }
        if (!first) {
            out += ",";
        }
        out += content;
        first = false;
    }

    out += "]";
    return out;
}

String SessionRecorder::summaryCsv() {
    String out;
    out.reserve(4096);
    out += SUMMARY_HEADER;

    std::vector<String> ids;
    collectIds(ids, ".sum.csv");
    std::sort(ids.begin(), ids.end(), [](const String &a, const String &b) { return a > b; });

    for (size_t i = 0; i < ids.size(); ++i) {
        File file = LittleFS.open(sumPath(ids[i]), "r");
        if (!file) {
            continue;
        }
        const String content = file.readString();
        file.close();
        if (out.length() + content.length() > 30000) {
            break;
        }
        out += content;
        if (!content.endsWith("\n")) {
            out += "\n";
        }
    }
    return out;
}

bool SessionRecorder::remove(const String &sessionId) {
    if (!mounted || !validId(sessionId)) {
        return false;
    }
    bool removed = false;
    removed = LittleFS.remove(csvPath(sessionId)) || removed;
    removed = LittleFS.remove(metaPath(sessionId)) || removed;
    removed = LittleFS.remove(sumPath(sessionId)) || removed;
    return removed;
}

bool SessionRecorder::removeAll() {
    if (!mounted) {
        return false;
    }

    std::vector<String> paths;
    File dir = LittleFS.open(AppConfig::SESSION_DIR);
    if (!dir) {
        return false;
    }
    File entry = dir.openNextFile();
    while (entry) {
        if (!entry.isDirectory()) {
            String name = basenameOf(entry.name());
            paths.push_back(String(AppConfig::SESSION_DIR) + "/" + name);
        }
        entry = dir.openNextFile();
    }
    dir.close();

    for (size_t i = 0; i < paths.size(); ++i) {
        LittleFS.remove(paths[i]);
    }
    return true;
}

size_t SessionRecorder::totalBytes() const {
    return mounted ? LittleFS.totalBytes() : 0;
}

size_t SessionRecorder::usedBytes() const {
    return mounted ? LittleFS.usedBytes() : 0;
}

size_t SessionRecorder::freeBytes() const {
    const size_t total = totalBytes();
    const size_t used = usedBytes();
    return total > used ? total - used : 0;
}

bool SessionRecorder::hasSpace() {
    return mounted && freeBytes() >= AppConfig::MIN_FREE_BYTES;
}
