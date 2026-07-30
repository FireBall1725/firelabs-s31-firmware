#include "Meter.h"
#include "Pins.h"

// Frame byte map (0-indexed, 24 bytes):
//   0      header / state (0x55 ok, 0xF. uncalibrated/error)
//   1      0x5A fixed marker
//   2..4   voltage coefficient      5..7   voltage cycle
//   8..10  current coefficient     11..13  current cycle
//   14..16 power coefficient       17..19  power cycle
//   20     adj flags (which cycle values are valid this frame)
//   21..22 CF pulse count (energy)
//   23     checksum = sum(bytes 2..22) & 0xFF
//
// adj bits (VERIFY against esphome): 0x40 voltage valid, 0x20 current valid,
// 0x10 power valid. If a "valid" bit is clear the load is below the chip's
// measurable floor and that quantity reads 0.
static const uint8_t ADJ_VOLTAGE = 0x40;
static const uint8_t ADJ_CURRENT = 0x20;
static const uint8_t ADJ_POWER   = 0x10;

void Meter::begin() {
  // One-shot probe: count toggles on the RX line before serial claims it. A
  // streaming CSE7766 produces hundreds of edges in 60ms; zero means no signal
  // on the wire (vs. a serial-config problem, which would show edges but rx=0).
  pinMode(PIN_CSE_RX, INPUT);
  int last = digitalRead(PIN_CSE_RX);
  uint32_t t0 = millis();
  while (millis() - t0 < 60) {
    int v = digitalRead(PIN_CSE_RX);
    if (v != last) { edges_++; last = v; }
  }

  Serial.setRxBufferSize(256);  // must precede begin() to take effect
  Serial.begin(4800, SERIAL_8E1);
}

void Meter::loop() {
  while (Serial.available()) {
    uint8_t b = (uint8_t)Serial.read();

    // Capture the raw stream (pre-framing) for the /api/meter diagnostic.
    raw_[rawIdx_] = b;
    rawIdx_ = (rawIdx_ + 1) % sizeof(raw_);
    rxBytes_++;

    // Frame sync. A real frame is header byte (0x55 or 0xF.) followed by the
    // fixed 0x5A marker. Requiring the 0x5A is what keeps us from locking onto
    // a 0x55/0xF. that just happens to appear inside the data.
    if (len_ == 0) {
      if (b == 0x55 || (b & 0xF0) == 0xF0) buf_[len_++] = b;  // candidate header
      continue;
    }
    if (len_ == 1) {
      if (b == 0x5A) buf_[len_++] = b;                        // confirmed
      else if (b == 0x55 || (b & 0xF0) == 0xF0) buf_[0] = b;  // resync to new header
      else len_ = 0;
      continue;
    }

    buf_[len_++] = b;
    if (len_ < 24) continue;

    uint8_t sum = 0;
    for (uint8_t i = 2; i <= 22; i++) sum += buf_[i];
    if (sum == buf_[23]) { frames_++; parseFrame(buf_); }
    len_ = 0;  // start hunting for the next header
  }
}

String Meter::rawHex() const {
  String s;
  char b[4];
  for (uint8_t k = 0; k < sizeof(raw_); k++) {
    snprintf(b, sizeof(b), "%02x ", raw_[(uint8_t)((rawIdx_ + k) % sizeof(raw_))]);
    s += b;
  }
  s.trim();
  return s;
}

void Meter::parseFrame(const uint8_t* f) {
  present_ = true;  // a valid frame means the CSE7766 is on the board
  const uint8_t adj = f[20];

  const uint32_t vCoef = be24(f + 2),  vCyc = be24(f + 5);
  const uint32_t iCoef = be24(f + 8),  iCyc = be24(f + 11);
  const uint32_t pCoef = be24(f + 14), pCyc = be24(f + 17);

  voltage_ = (adj & ADJ_VOLTAGE) && vCyc ? ((float)vCoef / vCyc) * calV_ : 0.0f;
  current_ = (adj & ADJ_CURRENT) && iCyc ? ((float)iCoef / iCyc) * calI_ : 0.0f;
  power_   = (adj & ADJ_POWER)   && pCyc ? ((float)pCoef / pCyc) * calP_ : 0.0f;

  sumV_ += voltage_;
  sumI_ += current_;
  sumP_ += power_;
  count_++;

  faultSumP_ += power_;
  faultCount_++;
}

bool Meter::sampleAverage(float& v, float& i, float& p) {
  if (count_ == 0) return false;
  v = (float)(sumV_ / count_);
  i = (float)(sumI_ / count_);
  p = (float)(sumP_ / count_);
  sumV_ = sumI_ = sumP_ = 0;
  count_ = 0;
  return true;
}

bool Meter::faultAverage(float& p) {
  if (faultCount_ == 0) return false;
  p = (float)(faultSumP_ / faultCount_);
  faultSumP_ = 0;
  faultCount_ = 0;
  return true;
}
