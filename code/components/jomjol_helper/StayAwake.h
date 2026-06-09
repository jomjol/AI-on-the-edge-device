#pragma once

// Persistent "Stay Awake" override for the SleepWhileIdle deep-sleep cycle.
//
// Stored in NVS so it survives reboots (including the cold-boot wake from
// deep sleep itself). Users toggle it via the dashboard button or the
// /sleep_override HTTP endpoint when they want the device to remain
// reachable for OTA updates / config changes.
//
// Honest USB detection is not possible on the AI-on-the-edge-cam hardware
// (TP4057 CHRG and STDBY pins drive only LEDs, no VBUS-sense GPIO), so
// this user-controlled flag is the reliable mechanism. See the
// SleepWhileIdle tooltip for the optional hardware mod that adds real
// USB detection.

// Read the current flag. Returns false on NVS error (fail-safe to normal
// sleep behaviour).
bool StayAwake_Get();

// Persist a new value. Returns true on success.
bool StayAwake_Set(bool value);
