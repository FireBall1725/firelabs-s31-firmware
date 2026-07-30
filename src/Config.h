#pragma once
#include <Arduino.h>

// All persisted settings, stored as one JSON document in LittleFS.
// Factory reset deletes the file and reboots into setup mode.
class Config {
public:
  enum class RestoreMode : uint8_t { Off, On, Last };

  // wifi
  String wifiSsid;
  String wifiPass;

  // identity (friendly name, e.g. "Living Room")
  String deviceName;

  // mqtt
  String mqttHost;
  uint16_t mqttPort = 1883;
  String mqttUser;
  String mqttPass;
  String discoveryPrefix = "homeassistant";

  // behavior
  RestoreMode restoreMode = RestoreMode::Last;
  bool relayLastState = false;   // remembered for RestoreMode::Last
  bool noLoadIndicator = false;
  uint8_t indicatorPercent = 30;

  // Fault (no-load) detection. Evaluated on a mean of the power samples, not a
  // single frame: an idle load's control board emits stray watt-level readings
  // that a one-shot comparison mistakes for the load running.
  bool faultEnabled = true;
  float faultWatts = 0.5f;        // mean power at or below this counts as no-load
  uint16_t faultHoldSec = 300;    // how long it must stay there before faulting

  // CSE7766 calibration multipliers (applied on top of datasheet scaling)
  float calVoltage = 1.0f;
  float calCurrent = 1.0f;
  float calPower = 1.0f;

  bool load();
  bool save() const;
  void clear();                       // factory reset
  bool hasWifi() const { return wifiSsid.length() > 0; }

  String hostname() const;            // "fl-living-room", sanitized
  String uniqueId() const;            // "fl-s31-e49d80", MAC-based, immutable
  String apSsid() const;              // "FireLabs S31 E49D80"
  String topicBase() const;           // "firelabs/fl-living-room"

  static String macSuffix();          // "E49D80", last 3 MAC octets

private:
  static const char* kPath;
};
