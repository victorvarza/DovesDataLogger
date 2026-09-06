///////////////////////////////////////////
// HEALTH LOG MODULE
// Periodic system-health snapshot to /HEALTH.csv (subsystem 19, informal):
// battery, GPS state, tach RPM, once a minute, independent of race sessions.
///////////////////////////////////////////

#include "health_log.h"

#define HEALTH_LOG_PATH "/HEALTH.csv"
#define HEALTH_LOG_INTERVAL_MS 10000UL

static unsigned long healthLogLastWriteMs = 0;
static File32 healthLogFile;

static const char* wakeCauseName(wake_cause::Cause c) {
  switch (c) {
    case wake_cause::Cause::kTachWake:       return "tach";
    case wake_cause::Cause::kButtonWake:     return "button";
    case wake_cause::Cause::kUsbWake:        return "usb";
    case wake_cause::Cause::kWatchdog:       return "watchdog";
    case wake_cause::Cause::kSoftReset:      return "soft_reset";
    case wake_cause::Cause::kColdBoot:       return "cold_boot";
    case wake_cause::Cause::kOffWakeUnknown: return "off_wake_unknown";
    default:                                 return "unknown";
  }
}

// Appends one line to HEALTH_LOG_PATH, creating it (with a CSV header) on
// first write. Uses SD_ACCESS_TRACK_PARSE, same as settings.ino — brief,
// same-task writes that nest under an in-progress LOGGING hold rather than
// taking exclusive ownership (see sd_access_policy).
static void healthLogAppendLine(const char* line) {
  if (!sdSetupSuccess) {
    debugln(F("Health log: sdSetupSuccess is false, skipped"));
    return;
  }
  if (!acquireSDAccess(SD_ACCESS_TRACK_PARSE)) {
    debugln(F("Health log: SD busy, skipped"));
    return;
  }

  bool isNewFile = !SD.exists(HEALTH_LOG_PATH);
  healthLogFile.open(HEALTH_LOG_PATH, O_WRITE | O_CREAT);
  if (!healthLogFile) {
    debugln(F("Health log: open failed"));
    releaseSDAccess(SD_ACCESS_TRACK_PARSE);
    return;
  }
  healthLogFile.seekEnd(0);
  if (isNewFile) {
    healthLogFile.println(F("uptime_s,battery_v,gps_fix,gps_time_valid,gps_rate_hz,gps_sats,tach_rpm,event"));
  }
  healthLogFile.println(line);
  healthLogFile.close();

  releaseSDAccess(SD_ACCESS_TRACK_PARSE);
  debug(F("Health log: wrote ["));
  debug(line);
  debugln(F("]"));
}

void HEALTH_SETUP() {
  char line[64];
  snprintf(line, sizeof(line), ",,,,,,,boot:wake=%s", wakeCauseName(bootWakeCause));
  healthLogAppendLine(line);
  healthLogLastWriteMs = millis();
}

void HEALTH_LOOP() {
  if (millis() - healthLogLastWriteMs < HEALTH_LOG_INTERVAL_MS) return;
  healthLogLastWriteMs = millis();

  char line[64];
  snprintf(line, sizeof(line), "%lu,%.2f,%d,%d,%.1f,%d,%d,",
           millis() / 1000UL, lastBatteryVoltage, gpsData.fix ? 1 : 0,
           gpsData.timeValid ? 1 : 0, gpsFrameRate, gpsData.satellites,
           tachLastReported);
  healthLogAppendLine(line);
}
