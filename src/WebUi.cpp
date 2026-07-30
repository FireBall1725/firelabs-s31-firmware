#include "WebUi.h"
#include "Config.h"
#include "Branding.h"
#include "Relay.h"
#include "Meter.h"
#include "AppPage.h"
#include "SetupPage.h"
#include "LogoPng.h"
#include <ESP8266WiFi.h>

void WebUi::begin(bool setupMode, Config* cfg, Relay* relay, Meter* meter) {
  cfg_ = cfg;
  relay_ = relay;
  meter_ = meter;
  // Re-begin (setup -> online after an in-place wifi retry, no reboot) must not
  // stack a second set of handlers on top of the captive-portal routes. reset()
  // drops the old handlers so we register exactly one face.
  if (begun_) server_.reset();
  routesCommon();
  if (setupMode) routesSetup();
  else routesOnline();
  if (!begun_) { server_.begin(); begun_ = true; }
}

void WebUi::routesCommon() {
  server_.on("/logo.png", HTTP_GET, [](AsyncWebServerRequest* req) {
    auto* r = req->beginResponse_P(200, "image/png", LOGO_PNG, LOGO_PNG_LEN);
    r->addHeader("Cache-Control", "max-age=604800");
    req->send(r);
  });
}

// Accumulate the request body, then parse + dispatch once it's complete.
void WebUi::handleJson(const char* path,
                       std::function<void(JsonDocument&, AsyncWebServerRequest*)> fn) {
  server_.on(path, HTTP_POST,
    [fn](AsyncWebServerRequest* req) {
      String* body = (String*)req->_tempObject;
      JsonDocument d;
      DeserializationError e = body ? deserializeJson(d, *body)
                                    : DeserializationError(DeserializationError::EmptyInput);
      if (e) req->send(400, "application/json", "{\"err\":\"bad json\"}");
      else   fn(d, req);
      if (body) { delete body; req->_tempObject = nullptr; }
    },
    nullptr,
    [](AsyncWebServerRequest* req, uint8_t* data, size_t len, size_t index, size_t total) {
      if (index == 0) req->_tempObject = new String();
      String* body = (String*)req->_tempObject;
      if (body) body->concat((const char*)data, len);
    });
}

// ---------- setup (AP) ----------

void WebUi::routesSetup() {
  auto serveSetup = [](AsyncWebServerRequest* req) {
    auto* r = req->beginResponse_P(200, "text/html", SETUP_HTML);
    r->addHeader("Cache-Control", "no-store");
    req->send(r);
  };
  server_.on("/", HTTP_GET, serveSetup);
  server_.onNotFound(serveSetup);  // captive-portal catch-all

  server_.on("/api/scan", HTTP_GET, [this](AsyncWebServerRequest* req) {
    // Cache only. Never scan here: a scan while a phone is on the SoftAP drops
    // it. The list was captured before the AP came up.
    req->send(200, "application/json", scanProvider_ ? scanProvider_() : String("[]"));
  });

  handleJson("/api/provision", [this](JsonDocument& d, AsyncWebServerRequest* req) {
    cfg_->wifiSsid   = (const char*)(d["ssid"] | "");
    cfg_->wifiPass   = (const char*)(d["pass"] | "");
    cfg_->deviceName = (const char*)(d["name"] | "");
    cfg_->save();
    req->send(200, "application/json", "{\"ok\":true}");
    restart_.once(2, []() { ESP.restart(); });  // reboot into station mode
  });

  handleOta();  // AP-mode firmware recovery, so a bad build isn't a cable job
}

// ---------- online ----------

void WebUi::routesOnline() {
  server_.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
    auto* r = req->beginResponse_P(200, "text/html", APP_HTML);
    r->addHeader("Cache-Control", "no-store");  // page changes with every OTA
    req->send(r);
  });

  server_.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest* req) {
    JsonDocument d;
    d["name"]    = cfg_->deviceName;
    d["host"]    = cfg_->hostname();
    d["mac"]     = WiFi.macAddress();
    d["model"]   = "S31";
    d["relay"]   = relay_->isOn();
    d["meter"]   = meter_->present();
    d["fault"]   = faultProvider_ ? faultProvider_() : false;
    d["noload"]  = cfg_->noLoadIndicator;
    d["restore"] = (uint8_t)cfg_->restoreMode;
    d["power"]   = meter_->power();
    d["voltage"] = meter_->voltage();
    d["current"] = meter_->current();
    d["energy"]  = energy_ ? energy_() : 0.0f;
    d["rssi"]    = WiFi.RSSI();
    d["uptime"]  = (uint32_t)(millis() / 1000);
    d["fw"]      = FL_FW_VERSION;
    d["flash"]   = ESP.getFlashChipRealSize();
    d["heap"]    = ESP.getFreeHeap();
    d["mqtt"]    = mqttUp_ ? mqttUp_() : false;
    d["topic"]   = cfg_->topicBase();
    d["mqtt_info"] = mqttInfo_ ? mqttInfo_() : String("");
    d["identify"] = identifyState_ ? identifyState_() : false;
    String out; serializeJson(d, out);
    req->send(200, "application/json", out);
  });

  handleJson("/api/identify", [this](JsonDocument& d, AsyncWebServerRequest* req) {
    if (onIdentify_) onIdentify_(d["on"] | false);
    req->send(200, "application/json", "{\"ok\":true}");
  });

  server_.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest* req) {
    JsonDocument d;
    d["mqtt_host"]    = cfg_->mqttHost;
    d["mqtt_port"]    = cfg_->mqttPort;
    d["disc_prefix"]  = cfg_->discoveryPrefix;
    d["mqtt_user"]    = cfg_->mqttUser;
    d["name"]         = cfg_->deviceName;
    d["restore_mode"] = (uint8_t)cfg_->restoreMode;
    d["noload"]       = cfg_->noLoadIndicator;
    d["brightness"]   = cfg_->indicatorPercent;
    d["fault_en"]     = cfg_->faultEnabled;
    d["fault_w"]      = cfg_->faultWatts;
    d["fault_hold"]   = cfg_->faultHoldSec;
    d["cal_p"]        = cfg_->calPower;
    String out; serializeJson(d, out);
    req->send(200, "application/json", out);
  });

  handleJson("/api/config", [this](JsonDocument& d, AsyncWebServerRequest* req) {
    if (d["mqtt_host"].is<const char*>())   cfg_->mqttHost = (const char*)d["mqtt_host"];
    if (d["mqtt_port"].is<int>())           cfg_->mqttPort = d["mqtt_port"];
    if (d["disc_prefix"].is<const char*>()) cfg_->discoveryPrefix = (const char*)d["disc_prefix"];
    if (d["mqtt_user"].is<const char*>())   cfg_->mqttUser = (const char*)d["mqtt_user"];
    if (d["mqtt_pass"].is<const char*>())   cfg_->mqttPass = (const char*)d["mqtt_pass"];
    if (d["name"].is<const char*>())        cfg_->deviceName = (const char*)d["name"];
    if (d["restore_mode"].is<int>())        cfg_->restoreMode = (Config::RestoreMode)(uint8_t)d["restore_mode"];
    if (d["noload"].is<bool>())             cfg_->noLoadIndicator = d["noload"];
    if (d["brightness"].is<int>())          cfg_->indicatorPercent = d["brightness"];
    if (d["fault_en"].is<bool>())           cfg_->faultEnabled = d["fault_en"];
    // is<float>() accepts ints too; is<int>() would reject 0.5 and silently drop
    // every fractional threshold.
    if (d["fault_w"].is<float>())
      cfg_->faultWatts = constrain((float)d["fault_w"], 0.0f, 100.0f);
    if (d["fault_hold"].is<int>())
      cfg_->faultHoldSec = (uint16_t)constrain((long)d["fault_hold"], 10L, 3600L);
    cfg_->save();
    if (onApply_) onApply_();
    req->send(200, "application/json", "{\"ok\":true}");
  });

  handleJson("/api/relay", [this](JsonDocument& d, AsyncWebServerRequest* req) {
    relay_->set(d["on"] | false);
    req->send(200, "application/json", "{\"ok\":true}");
  });

  handleJson("/api/calibrate", [this](JsonDocument& d, AsyncWebServerRequest* req) {
    float trueW = d["watts"] | 0.0f;
    float now = meter_->power();
    if (trueW > 0 && now > 0.5f) {
      cfg_->calPower = trueW * cfg_->calPower / now;  // rescale around current factor
      cfg_->save();
      if (onApply_) onApply_();
    }
    String out = String("{\"cal_p\":") + String(cfg_->calPower, 4) + "}";
    req->send(200, "application/json", out);
  });

  server_.on("/api/restart", HTTP_POST, [this](AsyncWebServerRequest* req) {
    req->send(200, "application/json", "{\"ok\":true}");
    restart_.once(1, []() { ESP.restart(); });
  });

  handleJson("/api/reset", [this](JsonDocument&, AsyncWebServerRequest* req) {
    cfg_->clear();
    req->send(200, "application/json", "{\"ok\":true}");
    restart_.once(1, []() { ESP.restart(); });
  });

  server_.on("/api/meter", HTTP_GET, [this](AsyncWebServerRequest* req) {
    JsonDocument d;
    d["present"] = meter_->present();
    d["edges"]   = meter_->lineEdges();
    d["rx"]      = meter_->rxBytes();
    d["frames"]  = meter_->frameCount();
    d["raw"]     = meter_->rawHex();
    String out; serializeJson(d, out);
    req->send(200, "application/json", out);
  });

  handleOta();
}

void WebUi::handleOta() {
  server_.on("/update", HTTP_POST,
    [this](AsyncWebServerRequest* req) {
      bool ok = !Update.hasError();
      req->send(200, "text/html", ok ? "Update OK, rebooting." : "Update failed.");
      if (ok) restart_.once(1, []() { ESP.restart(); });
    },
    [](AsyncWebServerRequest* req, String filename, size_t index, uint8_t* data, size_t len, bool final) {
      if (index == 0) {
        Update.runAsync(true);  // don't block the async server during flash write
        uint32_t maxSketch = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        Update.begin(maxSketch);
      }
      if (!Update.hasError()) Update.write(data, len);
      if (final) Update.end(true);
    });
}
