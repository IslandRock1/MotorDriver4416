//
// WiFi access point, captive portal and REST API for the power meter.
//

#include "WebApp.h"

#include <ESPmDNS.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <stdlib.h>

#include "AppConfig.h"
#include "WebPage.h"

namespace {

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

String boolJson(bool value) {
    return value ? "true" : "false";
}

} // namespace

WebApp::WebApp(PowerMeter &meterRef, Settings &settingsRef, SessionRecorder &recorderRef)
    : meter(meterRef), settings(settingsRef), recorder(recorderRef) {
}

void WebApp::begin() {
    WiFi.mode(WIFI_AP);
    WiFi.setSleep(false);

    const bool apStarted = WiFi.softAP(AppConfig::AP_SSID, AppConfig::AP_PASSWORD);
    const IPAddress ip = WiFi.softAPIP();
    Serial.printf("[wifi] AP \"%s\" %s, address http://%s\n", AppConfig::AP_SSID,
                  apStarted ? "started" : "FAILED", ip.toString().c_str());

    dns.start(53, "*", ip);

    if (MDNS.begin("barbell")) {
        MDNS.addService("http", "tcp", 80);
        Serial.println("[wifi] mDNS: http://barbell.local");
    }

    setupRoutes();
    server.begin();
    Serial.println("[web] HTTP server started on port 80");
}

void WebApp::handle() {
    dns.processNextRequest();
    server.handleClient();
}

void WebApp::setupRoutes() {
    server.on("/", HTTP_GET, [this]() { serveIndex(); });
    server.on("/index.html", HTTP_GET, [this]() { serveIndex(); });
    server.on("/favicon.ico", HTTP_GET, [this]() { server.send(204, "text/plain", ""); });

    server.on("/api/status", HTTP_GET, [this]() { handleStatus(); });
    server.on("/api/session/start", HTTP_POST, [this]() { handleSessionStart(); });
    server.on("/api/session/stop", HTTP_POST, [this]() { handleSessionStop(); });
    server.on("/api/settings", HTTP_POST, [this]() { handleSettings(); });
    server.on("/api/calibrate/start", HTTP_POST, [this]() { handleCalibrationStart(); });
    server.on("/api/calibrate/finish", HTTP_POST, [this]() { handleCalibrationFinish(); });
    server.on("/api/calibrate/cancel", HTTP_POST, [this]() { handleCalibrationCancel(); });
    server.on("/api/takeup/autodir", HTTP_POST, [this]() { handleAutoDirection(); });
    server.on("/api/zero", HTTP_POST, [this]() { handleZero(); });
    server.on("/api/flip-direction", HTTP_POST, [this]() { handleFlipDirection(); });
    server.on("/api/sessions", HTTP_GET, [this]() { handleSessions(); });
    server.on("/api/sessions/download", HTTP_GET, [this]() { handleDownload(); });
    server.on("/api/sessions/metadata", HTTP_GET, [this]() { handleDownloadMeta(); });
    server.on("/api/sessions/delete", HTTP_POST, [this]() { handleDelete(); });
    server.on("/api/sessions/deleteAll", HTTP_POST, [this]() { handleDeleteAll(); });
    server.on("/api/export/summary.csv", HTTP_GET, [this]() { handleSummaryCsv(); });
    server.on("/api/reboot", HTTP_POST, [this]() { handleReboot(); });

    server.onNotFound([this]() { handleNotFound(); });
}

void WebApp::serveIndex() {
    server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate");
    server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
}

void WebApp::sendJson(const String &json) {
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json; charset=utf-8", json);
}

void WebApp::sendError(int code, const String &message) {
    String body = "{\"ok\":false,\"error\":\"";
    body += jsonEscape(message);
    body += "\"}";
    server.sendHeader("Cache-Control", "no-store");
    server.send(code, "application/json; charset=utf-8", body);
}

String WebApp::summaryJson(const PowerMeter::Summary &summary) const {
    String j;
    j.reserve(420);
    j += "{\"valid\":";
    j += boolJson(summary.valid);
    if (summary.valid) {
        j += ",\"id\":\"" + jsonEscape(summary.id) + "\"";
        j += ",\"epoch\":" + String(summary.epoch);
        j += ",\"exercise\":\"" + jsonEscape(summary.exercise) + "\"";
        j += ",\"weightKg\":" + String(summary.weightKg, 2);
        j += ",\"reps\":" + String(summary.reps);
        j += ",\"durationMs\":" + String(summary.durationMs);
        j += ",\"avgPowerW\":" + String(summary.avgPower, 1);
        j += ",\"peakPowerW\":" + String(summary.peakPower, 1);
        j += ",\"avgVelocityMps\":" + String(summary.avgVelocity, 3);
        j += ",\"peakVelocityMps\":" + String(summary.peakVelocity, 3);
        j += ",\"romCm\":" + String(summary.romCm, 1);
        j += ",\"workJ\":" + String(summary.workJ, 1);
        j += ",\"tutMs\":" + String(summary.tutMs);
        j += ",\"calibrated\":";
        j += boolJson(summary.calibrated);
    }
    j += "}";
    return j;
}

String WebApp::statusJson() {
    const PowerMeter::Snapshot snapshot = meter.snapshot(millis());
    String j;
    j.reserve(3400);

    j += "{\"ok\":true";
    j += ",\"device\":{\"uptimeMs\":" + String(millis());
    j += ",\"freeHeap\":" + String(ESP.getFreeHeap());
    j += ",\"ip\":\"" + WiFi.softAPIP().toString() + "\"";
    j += ",\"ssid\":\"" + String(AppConfig::AP_SSID) + "\"}";

    j += ",\"storage\":{\"available\":";
    j += boolJson(recorder.available());
    j += ",\"total\":" + String(static_cast<unsigned long>(recorder.totalBytes()));
    j += ",\"used\":" + String(static_cast<unsigned long>(recorder.usedBytes()));
    j += ",\"free\":" + String(static_cast<unsigned long>(recorder.freeBytes()));
    j += "}";

    j += ",\"calibration\":{\"active\":";
    j += boolJson(snapshot.calibrating);
    j += ",\"counts\":" + String(static_cast<long>(snapshot.calibrationCounts));
    j += ",\"countsPerMeter\":" + String(snapshot.countsPerMeter, 1);
    j += ",\"calibrated\":";
    j += boolJson(snapshot.calibrated);
    j += ",\"extensionSign\":" + String(static_cast<int>(snapshot.extensionSign));
    j += "}";

    j += ",\"settings\":{\"takeupEnabled\":";
    j += boolJson(snapshot.takeupEnabled);
    j += ",\"takeupPercent\":" + String(static_cast<unsigned>(snapshot.takeupPercent));
    j += ",\"takeupDir\":";
    j += boolJson(snapshot.takeupDir);
    j += ",\"minRepRomCm\":" + String(snapshot.minRepRomCm, 1);
    j += ",\"velocityDeadband\":" + String(snapshot.velocityDeadband, 2);
    j += ",\"lastWeightKg\":" + String(settings.lastWeightKg, 2);
    j += ",\"lastExercise\":\"" + jsonEscape(settings.lastExercise) + "\"}";

    j += ",\"live\":{\"positionM\":" + String(snapshot.positionM, 4);
    j += ",\"velocity\":" + String(snapshot.velocity, 3);
    j += ",\"power\":" + String(snapshot.power, 1);
    j += ",\"raw\":" + String(static_cast<long>(snapshot.rawPosition)) + "}";

    j += ",\"session\":{\"active\":";
    j += boolJson(snapshot.active);
    j += ",\"id\":\"" + jsonEscape(snapshot.id) + "\"";
    j += ",\"recording\":";
    j += boolJson(snapshot.recording);
    j += ",\"exercise\":\"" + jsonEscape(snapshot.exercise) + "\"";
    j += ",\"weightKg\":" + String(snapshot.weightKg, 2);
    j += ",\"elapsedMs\":" + String(snapshot.elapsedMs);
    j += ",\"reps\":" + String(snapshot.reps);
    j += ",\"phase\":" + String(static_cast<unsigned>(snapshot.phase));
    j += ",\"power\":" + String(snapshot.power, 1);
    j += ",\"peakPower\":" + String(snapshot.peakPower, 1);
    j += ",\"avgPower\":" + String(snapshot.avgPower, 1);
    j += ",\"romCm\":" + String(snapshot.romCm, 1);
    j += ",\"workJ\":" + String(snapshot.workJ, 1);
    j += ",\"tutMs\":" + String(snapshot.tutMs);
    j += "}";

    j += ",\"lastReps\":[";
    for (uint8_t i = 0; i < snapshot.lastRepCount; ++i) {
        if (i > 0) {
            j += ",";
        }
        const PowerMeter::RepInfo &r = snapshot.lastReps[i];
        j += "{\"i\":" + String(r.index);
        j += ",\"romCm\":" + String(r.romCm, 1);
        j += ",\"ms\":" + String(r.durationMs);
        j += ",\"avgV\":" + String(r.avgVelocity, 3);
        j += ",\"peakV\":" + String(r.peakVelocity, 3);
        j += ",\"avgW\":" + String(r.avgPower, 1);
        j += ",\"peakW\":" + String(r.peakPower, 1);
        j += ",\"workJ\":" + String(r.workJ, 1);
        j += "}";
    }
    j += "]";

    j += ",\"lastSummary\":";
    j += summaryJson(snapshot.lastSummary);
    j += "}";
    return j;
}

void WebApp::handleStatus() {
    sendJson(statusJson());
}

void WebApp::handleSessionStart() {
    if (!server.hasArg("weight")) {
        sendError(400, "Missing weight.");
        return;
    }

    const float weight = server.arg("weight").toFloat();
    String exercise = server.arg("exercise");
    if (exercise == "Custom" && server.hasArg("custom")) {
        exercise = server.arg("custom");
    }
    if (exercise.length() == 0) {
        exercise = "Custom";
    }

    uint32_t epoch = 0;
    if (server.hasArg("epoch")) {
        epoch = static_cast<uint32_t>(strtoul(server.arg("epoch").c_str(), nullptr, 10));
    }

    String error;
    if (!meter.startSession(weight, exercise, epoch, error)) {
        sendError(400, error);
        return;
    }
    sendJson(statusJson());
}

void WebApp::handleSessionStop() {
    if (!meter.stopSession()) {
        sendError(400, "No set is running.");
        return;
    }
    sendJson(statusJson());
}

void WebApp::handleSettings() {
    if (server.hasArg("takeupEnabled")) {
        const String value = server.arg("takeupEnabled");
        meter.setTakeupEnabled(value == "1" || value == "true");
    }
    if (server.hasArg("takeupPercent")) {
        long percent = server.arg("takeupPercent").toInt();
        if (percent < 0) percent = 0;
        if (percent > 100) percent = 100;
        meter.setTakeupStrength(static_cast<uint8_t>(percent));
    }
    if (server.hasArg("takeupDir")) {
        meter.setTakeupDir(server.arg("takeupDir") == "1");
    }
    if (server.hasArg("minRepRomCm")) {
        float value = server.arg("minRepRomCm").toFloat();
        if (value < 1.0f) value = 1.0f;
        if (value > 100.0f) value = 100.0f;
        settings.minRepRomCm = value;
        settings.save();
    }
    if (server.hasArg("velocityDeadband")) {
        float value = server.arg("velocityDeadband").toFloat();
        if (value < 0.0f) value = 0.0f;
        if (value > 0.5f) value = 0.5f;
        settings.velocityDeadband = value;
        settings.save();
    }
    sendJson(statusJson());
}

void WebApp::handleCalibrationStart() {
    String error;
    if (!meter.startCalibration(error)) {
        sendError(400, error);
        return;
    }
    sendJson(statusJson());
}

void WebApp::handleCalibrationFinish() {
    String message;
    if (!meter.finishCalibration(message)) {
        sendError(400, message);
        return;
    }
    String body = "{\"ok\":true,\"message\":\"" + jsonEscape(message) + "\"}";
    sendJson(body);
}

void WebApp::handleCalibrationCancel() {
    meter.cancelCalibration();
    sendJson("{\"ok\":true}");
}

void WebApp::handleAutoDirection() {
    String message;
    bool newDir = false;
    if (!meter.autoDetectTakeupDirection(message, newDir)) {
        sendError(400, message);
        return;
    }
    String body = "{\"ok\":true,\"message\":\"" + jsonEscape(message);
    body += "\",\"dir\":";
    body += newDir ? "1" : "0";
    body += "}";
    sendJson(body);
}

void WebApp::handleZero() {
    meter.zeroPosition();
    sendJson(statusJson());
}

void WebApp::handleFlipDirection() {
    meter.flipLiftDirection();
    sendJson(statusJson());
}

void WebApp::handleSessions() {
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json; charset=utf-8", recorder.listJson());
}

void WebApp::handleDownload() {
    const String sessionId = server.arg("id");
    if (!SessionRecorder::validId(sessionId)) {
        sendError(400, "Invalid session id.");
        return;
    }

    if (meter.sessionActive() && sessionId == meter.snapshot(millis()).id) {
        sendError(409, "Stop the set before downloading it.");
        return;
    }

    const String path = String(AppConfig::SESSION_DIR) + "/" + sessionId + ".csv";
    File file = LittleFS.open(path, "r");
    if (!file) {
        sendError(404, "Session not found.");
        return;
    }

    server.sendHeader("Content-Disposition", "attachment; filename=\"" + sessionId + ".csv\"");
    server.sendHeader("Cache-Control", "no-store");
    server.streamFile(file, "text/csv; charset=utf-8");
    file.close();
}

void WebApp::handleDownloadMeta() {
    const String sessionId = server.arg("id");
    if (!SessionRecorder::validId(sessionId)) {
        sendError(400, "Invalid session id.");
        return;
    }

    if (meter.sessionActive() && sessionId == meter.snapshot(millis()).id) {
        sendError(409, "Stop the set before downloading it.");
        return;
    }

    const String path = String(AppConfig::SESSION_DIR) + "/" + sessionId + ".json";
    File file = LittleFS.open(path, "r");
    if (!file) {
        sendError(404, "Session not found.");
        return;
    }

    server.sendHeader("Content-Disposition", "attachment; filename=\"" + sessionId + ".json\"");
    server.sendHeader("Cache-Control", "no-store");
    server.streamFile(file, "application/json; charset=utf-8");
    file.close();
}

void WebApp::handleDelete() {
    if (meter.sessionActive()) {
        sendError(409, "Stop the current set before deleting sessions.");
        return;
    }
    const String sessionId = server.arg("id");
    if (!SessionRecorder::validId(sessionId)) {
        sendError(400, "Invalid session id.");
        return;
    }
    if (!recorder.remove(sessionId)) {
        sendError(404, "Session not found.");
        return;
    }
    sendJson("{\"ok\":true}");
}

void WebApp::handleDeleteAll() {
    if (meter.sessionActive()) {
        sendError(409, "Stop the current set before deleting sessions.");
        return;
    }
    recorder.removeAll();
    sendJson("{\"ok\":true}");
}

void WebApp::handleSummaryCsv() {
    const String csv = recorder.summaryCsv();
    server.sendHeader("Content-Disposition", "attachment; filename=\"barbell_summary.csv\"");
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "text/csv; charset=utf-8", csv);
}

void WebApp::handleReboot() {
    if (meter.sessionActive()) {
        meter.stopSession();
    }
    sendJson("{\"ok\":true}");
    delay(250);
    ESP.restart();
}

void WebApp::handleNotFound() {
    if (server.uri().startsWith("/api/")) {
        sendError(404, "Not found.");
        return;
    }

    const String location = "http://" + WiFi.softAPIP().toString() + "/";
    server.sendHeader("Location", location, true);
    server.sendHeader("Cache-Control", "no-store");
    server.send(302, "text/plain", "");
}
