#ifndef _DOVES_HEALTH_LOG_H
#define _DOVES_HEALTH_LOG_H

// Periodic system-health snapshot to /HEALTH.csv, independent of any race
// session — battery voltage, GPS fix/time-lock/frame-rate/satellite count,
// and tach RPM, once a minute. One line per boot marks the wake cause so a
// long-running card shows a history across power cycles.

void HEALTH_SETUP();
void HEALTH_LOOP();

#endif
