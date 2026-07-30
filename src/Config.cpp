#include "Config.h"
#include "Branding.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

const char* Config::kPath = "/config.json";

String Config::macSuffix() {
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char buf[7];
  snprintf(buf, sizeof(buf), "%02X%02X%02X", mac[3], mac[4], mac[5]);
  return String(buf);
}

static String sanitizeHost(const String& in) {
  String out;
  for (size_t i = 0; i < in.length(); i++) {
    char c = in[i];
    if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
    if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')) out += c;
    else if (c == ' ' || c == '-' || c == '_') {
      if (out.length() && out[out.length() - 1] != '-') out += '-';
    }
  }
  while (out.length() && out[out.length() - 1] == '-') out.remove(out.length() - 1);
  return out;
}

String Config::hostname() const {
  String s = sanitizeHost(deviceName);
  if (s.length() == 0) return String(FL_HOST_PREFIX "-s31-") + macSuffix();  // fallback
  return String(FL_HOST_PREFIX "-") + s;
}

String Config::uniqueId() const {
  String suffix = macSuffix();
  suffix.toLowerCase();
  return String(FL_HOST_PREFIX "-s31-") + suffix;
}

String Config::apSsid() const {
  return String(FL_AP_PREFIX " ") + macSuffix();
}

String Config::topicBase() const {
  return String(FL_MQTT_TOPIC_ROOT "/") + hostname();
}

bool Config::load() {
  if (!LittleFS.begin()) return false;
  File f = LittleFS.open(kPath, "r");
  if (!f) return false;

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, f);
  f.close();
  if (err) return false;

  wifiSsid         = doc["wifi_ssid"]   | "";
  wifiPass         = doc["wifi_pass"]   | "";
  deviceName       = doc["name"]        | "";
  mqttHost         = doc["mqtt_host"]   | "";
  mqttPort         = doc["mqtt_port"]   | 1883;
  mqttUser         = doc["mqtt_user"]   | "";
  mqttPass         = doc["mqtt_pass"]   | "";
  discoveryPrefix  = doc["disc_prefix"] | "homeassistant";
  restoreMode      = (RestoreMode)(uint8_t)(doc["restore_mode"] | 2);
  relayLastState   = doc["relay_last"]  | false;
  noLoadIndicator  = doc["noload_ind"]  | false;
  indicatorPercent = doc["ind_pct"]     | 30;
  faultEnabled     = doc["flt_en"]      | true;
  faultWatts       = doc["flt_w"]       | 0.5f;
  faultHoldSec     = doc["flt_hold"]    | 300;
  calVoltage       = doc["cal_v"]       | 1.0f;
  calCurrent       = doc["cal_i"]       | 1.0f;
  calPower         = doc["cal_p"]       | 1.0f;
  return true;
}

bool Config::save() const {
  if (!LittleFS.begin()) return false;
  JsonDocument doc;
  doc["wifi_ssid"]    = wifiSsid;
  doc["wifi_pass"]    = wifiPass;
  doc["name"]         = deviceName;
  doc["mqtt_host"]    = mqttHost;
  doc["mqtt_port"]    = mqttPort;
  doc["mqtt_user"]    = mqttUser;
  doc["mqtt_pass"]    = mqttPass;
  doc["disc_prefix"]  = discoveryPrefix;
  doc["restore_mode"] = (uint8_t)restoreMode;
  doc["relay_last"]   = relayLastState;
  doc["noload_ind"]   = noLoadIndicator;
  doc["ind_pct"]      = indicatorPercent;
  doc["flt_en"]       = faultEnabled;
  doc["flt_w"]        = faultWatts;
  doc["flt_hold"]     = faultHoldSec;
  doc["cal_v"]        = calVoltage;
  doc["cal_i"]        = calCurrent;
  doc["cal_p"]        = calPower;

  File f = LittleFS.open(kPath, "w");
  if (!f) return false;
  serializeJson(doc, f);
  f.close();
  return true;
}

void Config::clear() {
  if (LittleFS.begin()) LittleFS.remove(kPath);
}
