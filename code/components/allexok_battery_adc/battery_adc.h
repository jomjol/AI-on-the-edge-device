#pragma once

#ifdef BOARD_ESP32_S3_ALEKSEI

// Battery voltage monitoring for the AI-on-the-edge-cam.
//
// Hardware: GPIO2 (ADC1_CH1) reads a 10k/10k divider tapping the Li-ion
// Vbatt rail directly (always-on, ~210 uA continuous). A Schottky diode
// (D7) bridges +5Vin into Vbatt, so when USB is plugged in the rail
// reads ~4.6 V and the divider reads ~2.3 V at the ADC -- above any
// real cell voltage. We detect that case via the USB-backfeed threshold
// and clamp the reported percent.

constexpr float LIION_EMPTY_V                  = 3.00f;
constexpr float LIION_FULL_V                   = 4.20f;
constexpr float LIION_USB_BACKFEED_THRESHOLD_V = 4.25f;
constexpr float BAT_DIVIDER_RATIO              = 2.0f;  // R24 (10k) + R34 (10k)


// Initialise the ADC and calibration. Idempotent. Returns true on success.
bool Battery_Init();

// True once Battery_Init() has succeeded.
bool Battery_IsReady();

// Read Vbatt in volts via a robust 7-block / 41-sample median-of-medians
// filter. Returns -1.0f if not initialised or the read failed. Side-effect:
// updates the last-read accessors below.
float Battery_ReadVoltage();

// Most recent median raw ADC value (0-4095) from Battery_ReadVoltage().
// -1 if no successful read yet.
int Battery_LastRawAdc();

// Most recent ADC-side voltage in volts (before the *2 divider scale).
// -1.0f if no successful read yet.
float Battery_LastAdcVoltage();

// Map a Vbatt reading to an estimated 0-100% on a piecewise-linear Li-ion
// curve. Above LIION_USB_BACKFEED_THRESHOLD_V the cell is assumed to be
// on external power and 100 is returned.
int Battery_PercentFromVoltage(float v);

// Legacy entry point used by ClassFlowLorawan. Returns Vbatt in
// millivolts (post-divider, i.e. the actual battery voltage). Lazy-inits
// on first call so it works regardless of whether Battery.Enabled is set.
// Returns -1 on failure.
int readBattery();

#endif // BOARD_ESP32_S3_ALEKSEI
