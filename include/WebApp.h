//
// WiFi access point, captive portal and REST API for the power meter.
//

#ifndef WEBAPP_H
#define WEBAPP_H

#include <Arduino.h>
#include <DNSServer.h>
#include <WebServer.h>

#include "PowerMeter.h"
#include "SessionRecorder.h"
#include "Settings.h"

class WebApp {
public:
    WebApp(PowerMeter &meter, Settings &settings, SessionRecorder &recorder);

    void begin();
    void handle();

private:
    void setupRoutes();
    void serveIndex();

    void handleStatus();
    void handleSessionStart();
    void handleSessionStop();
    void handleSettings();
    void handleCalibrationStart();
    void handleCalibrationFinish();
    void handleCalibrationCancel();
    void handleAutoDirection();
    void handleZero();
    void handleFlipDirection();
    void handleSessions();
    void handleDownload();
    void handleDownloadMeta();
    void handleDelete();
    void handleDeleteAll();
    void handleSummaryCsv();
    void handleReboot();
    void handleNotFound();

    void sendJson(const String &json);
    void sendError(int code, const String &message);

    String statusJson();
    String summaryJson(const PowerMeter::Summary &summary) const;

    PowerMeter &meter;
    Settings &settings;
    SessionRecorder &recorder;

    WebServer server{80};
    DNSServer dns;
};

#endif // WEBAPP_H
