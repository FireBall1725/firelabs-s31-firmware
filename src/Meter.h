#pragma once
#include <Arduino.h>

// CSE7766 power meter on UART0 RX (GPIO3), 4800 8E1, receive only.
// Parses the 24-byte frame, applies datasheet scaling, then the user
// calibration multipliers. Keeps a running average so the publisher can
// throttle output (SPEC.md: 8s default).
//
// Frame layout and adj-bit semantics follow the CSE7766 protocol as used by
// ESPHome/Tasmota. VERIFY the adj-flag masks against esphome cse7766.cpp before
// trusting readings; they are the easiest thing to get subtly wrong.
class Meter {
public:
  void begin();
  void loop();   // call often; drains UART and parses frames

  void setCalibration(float v, float i, float p) { calV_ = v; calI_ = i; calP_ = p; }

  // True once any valid frame has been parsed. An S31 Lite has no CSE7766, so
  // this never latches and the rest of the firmware hides power features.
  bool present() const { return present_; }

  // Latest instantaneous values (already calibrated).
  float voltage() const { return voltage_; }
  float current() const { return current_; }
  float power()   const { return power_; }

  // Averaged sample since the last call; resets the accumulator. Returns false
  // if no frames arrived in the window.
  bool sampleAverage(float& v, float& i, float& p);

  // Mean power since the last call, on an accumulator of its own so the fault
  // detector and the publisher can each consume a full window. Returns false if
  // no frames arrived; an empty window must not be judged either way.
  bool faultAverage(float& p);

  // Diagnostics for the /api/meter endpoint.
  uint32_t rxBytes() const { return rxBytes_; }
  uint32_t frameCount() const { return frames_; }
  uint32_t lineEdges() const { return edges_; }  // toggles seen on the RX line at boot
  String rawHex() const;

private:
  void parseFrame(const uint8_t* f);
  static uint32_t be24(const uint8_t* p) { return ((uint32_t)p[0] << 16) | ((uint32_t)p[1] << 8) | p[2]; }

  uint8_t buf_[24];
  uint8_t len_ = 0;

  bool present_ = false;
  float voltage_ = 0, current_ = 0, power_ = 0;
  float calV_ = 1, calI_ = 1, calP_ = 1;

  // raw stream capture for diagnostics
  uint8_t raw_[32] = {0};
  uint8_t rawIdx_ = 0;
  uint32_t rxBytes_ = 0;
  uint32_t frames_ = 0;
  uint32_t edges_ = 0;

  // accumulators for averaging
  double sumV_ = 0, sumI_ = 0, sumP_ = 0;
  uint32_t count_ = 0;

  // second, independent power accumulator drained by the fault detector
  double faultSumP_ = 0;
  uint32_t faultCount_ = 0;
};
