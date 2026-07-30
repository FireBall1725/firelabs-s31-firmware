# FireLabs S31 Firmware Spec

Custom firmware for the Sonoff S31 smart plug. Replaces the stock Sonoff firmware
(or ESPHome, if you flashed that on yourself) with an MQTT-native firmware that
talks to Home Assistant through MQTT Discovery, and carries its own web UI for
setup and configuration.

Author: FireBall1725

## Goal

Move off ESPHome's native API onto MQTT, keep a small web UI for provisioning and
config, and own the firmware so it can be extended later. MQTT Discovery gives
native HA entities out of the box; a companion HACS integration (see below) covers
setups that don't run MQTT.

## Hardware

This unit is the separate-flash ESP8266 variant, not the ESP8285. Confirmed at
runtime: `ESP.getFlashChipRealSize()` returned `4194304` (4MB), Zbit ZB25VQ32
chip. MAC `C4:5B:BE:E4:9D:80`.

| Pin | Function | Notes |
|---|---|---|
| GPIO0 | Button | INPUT_PULLUP, active low; also the boot-strap pin (low at power-on = flash mode) |
| GPIO12 | Relay | Also drives the red LED in hardware; red = relay state, not software-controllable |
| GPIO13 | Blue LED | PWM-capable, active low; the only software-controllable LED |
| GPIO3 (RX) | CSE7766 data | 4800 baud, 8E1, receive only; the chip streams, we never transmit |
| GPIO1 (TX) | UART0 TX | Wired in the metering circuit; not a free debug pin |

Two hardware facts shape the design:

1. **No serial console.** UART0 RX is consumed by the CSE7766, so logging goes
   over the network and the web UI, never over the cable. The ESPHome config set
   `baud_rate: 0` for the same reason.
2. **The button is the boot pin.** A runtime long-press reset is fine; a
   hold-during-power-on drops the chip into the bootloader instead of booting, so
   the reset logic is runtime-only.

RAM is the binding constraint, not flash. About 80KB total, roughly 40KB free
heap with the web server and MQTT running. That rules out TLS MQTT in practice
(the handshake wants ~20KB of heap), so MQTT runs plaintext on the trusted LAN.

## Firmware stack

- Arduino framework on ESP8266, built with PlatformIO.
- Board set to `esp12e` so the firmware sees all 4MB. The old config used
  `esp01_1m`, which capped it at 1MB.
- Async throughout: ESPAsyncWebServer + AsyncMqttClient + ESPAsyncTCP, so a
  stalled broker can't freeze the web UI and a busy web UI can't stall MQTT.
- LittleFS for config and web assets.
- ArduinoOTA / web OTA for updates. 4MB leaves comfortable room for two app
  slots plus a filesystem.

## Config storage

A single JSON document in LittleFS holds all settings: wifi credentials, device
name, MQTT broker config, relay restore mode, no-load indicator on/off, and
CSE7766 calibration constants. Factory reset deletes this file and reboots into
setup mode.

## Provisioning flow

First boot with no stored wifi starts a SoftAP captive portal.

1. Broadcast open AP `FireLabs S31 E49D80` (last three MAC octets).
2. A DNS server on the device answers every lookup with its own IP, which trips
   the phone's captive-portal detector (iOS hits captive.apple.com, Android its
   connectivity check) and opens the wizard automatically. A catch-all route on
   ESPAsyncWebServer serves the page for any path.
3. The wizard scans visible networks, takes the chosen SSID, its password, and
   the device name.
4. The device writes config, switches to station mode, and connects.

Two constraints the flow has to respect:

- **The phone loses the device when it leaves AP mode.** The wizard can't show a
  post-reboot success link, because nothing is listening at the old address once
  the AP drops. So the last wizard page states the target address up front:
  "this plug will be at http://fl-<name>.local and in your router as fl-<name>."
- **Fallback to AP.** If the device can't join within 30s (wrong password, wifi
  moved), it reopens the setup AP instead of going dark. That keeps a
  misconfigured plug off the workbench.

mDNS (`fl-<name>.local`) is advertised but not load-bearing. It works on Apple,
is inconsistent on Android, and needs Bonjour on Windows. Once MQTT is set up,
Home Assistant finds the plug through MQTT Discovery, not mDNS, so `.local` only
matters for reaching the admin web UI.

## Naming model

Four separate identifiers. Keeping them separate is what prevents orphaned HA
entities on a rename.

| Layer | Example | Rule |
|---|---|---|
| AP SSID | `FireLabs S31 E49D80` | MAC suffix, fixed |
| Hostname / mDNS | `fl-living-room` | the typed name, sanitized: lowercase, spaces to hyphens, alphanumeric and hyphen only |
| HA friendly name | `Living Room` | freeform, whatever the user types |
| Unique ID (MQTT) | `fl-s31-e49d80` | MAC-based, immutable |

The unique_id never derives from the typed name. If it did, renaming would make
HA drop the old entities and lose their history. MAC-based unique_id means the
name is just a label and history survives a rename. MQTT topics namespace under
`firelabs/fl-<name>/...`.

## Web UI

The wizard collects only what gets the plug online and findable: wifi plus device
name. Everything else lives in a settings UI reached at `fl-<name>.local` after
the plug joins the network, on these tabs:

- **Status:** relay state, a fault banner when the load has stopped drawing
  power, live power/voltage/current, wifi signal, uptime, firmware version.
- **MQTT:** broker host, port, username, password, discovery prefix.
- **Settings:** relay restore mode, no-load indicator on/off, indicator
  brightness, fault detection on/off with its threshold and dwell.
- **Calibration:** single-point CSE7766 calibration against a known load, with
  step-by-step directions on the page (see below).
- **System:** OTA upload, restart, factory reset.

Reason for the split: broker details aren't usually on hand at the outlet, and a
real browser beats thumbing through a captive portal.

## MQTT and Home Assistant

MQTT Discovery publishes the entity configs so HA auto-creates the device and its
entities, grouped under one device record keyed by the MAC-based unique_id.
Entities, matching the ESPHome feature set so nothing regresses:

- Switch: relay
- Sensors: power (W), voltage (V), current (A), total daily energy (kWh),
  wifi signal (dBm), uptime (s)
- Binary sensors: status/availability, button, fault (no-load)
- Diagnostic: firmware version, flash size
- Config: no-load indicator enable, relay restore mode

Power, voltage, and current publish on a throttle (default 8s) so the broker
isn't flooded; the CSE7766 streams several readings per second. Total daily
energy integrates the power figure.

## LED behavior

Blue is a single channel several states want, so it runs on a priority order, not
independent toggles. Highest active condition wins.

| Priority | Condition | Pattern |
|---|---|---|
| 1 | AP setup mode | Slow blink, 1s on / 1s off, full brightness |
| 2 | Connecting to wifi | Fast blink, ~3Hz, full brightness |
| 3 | No-load indicator (fault detected), if enabled | Solid, dimmed default |
| 4 | Normal | Off |

The two blink states run full brightness because they're transient and you're
looking at the plug during setup or a connect attempt. The no-load indicator runs
dimmed (~30% default, adjustable) because it can sit on for hours. It tracks the
fault state described in the next section. Red on plus blue on reads as "powered
but nothing drawing"; red alone reads as "powered and working."

The no-load indicator defaults off (opt-in).

## Fault detection

A fault means the relay is on and the load has stopped drawing power: a humidifier
that ran dry, a heater that tripped its thermal cutout, an appliance someone
switched off at its own button.

The test runs on the mean of the power samples collected over a 5s window, never
on the latest CSE7766 frame. Two things make a single reading useless here. An
idle appliance's control board still emits occasional watt-level frames, measured
at up to 1.9W on a humidifier drawing 0.03A, and one comparison against a low
threshold reads those as the load running. Going the other way, an appliance that
duty-cycles drops to a true 0W for seconds at a stretch while working correctly,
so a low reading on its own proves nothing either.

Defaults are 0.5W and 300s, both adjustable per device on the Settings tab because
the right numbers depend on the load. On the two humidifiers this was tuned
against, the idle 5-minute mean sits between 0.04W and 0.12W while the running
5-minute mean never drops below 2.69W, which puts 0.5W roughly 5x clear of each.

The dwell is deliberately asymmetric. Faulting takes the full configured hold;
clearing takes 15s above the threshold. Slow to fault so a duty cycle can't trip
it, quicker to clear so a load coming back gets noticed without a five-minute lag.
Switching the relay off clears the fault at once, since an off relay isn't a
fault.

An S31 Lite has no CSE7766, so it never computes or publishes a fault at all.

## Button behavior

GPIO0 carries two actions; tap versus hold disambiguates them.

- **Release under 1s:** toggle the relay. This moves the toggle to fire on
  release, not on press as the ESPHome config did, so a hold doesn't both toggle
  and reset.
- **Hold 1s to 5s:** reset is arming. Blue starts a slow blink and accelerates
  continuously toward 5s.
- **Hold reaches 5s:** blue goes solid (committed), then plays the confirmation:
  off ~2s, then 5 quick flashes, then clear config and reboot into setup AP.
- **Release between 1s and 5s:** abort. No reset and no relay toggle; the LED
  returns to its prior state.

## Relay

Restore-on-boot defaults to last known state, persisted to flash (the old
`restore_from_flash` behavior). During flashing the relay is forced off, per the
programmer warning on the device page. Restore mode is configurable on the
Settings tab.

## CSE7766 calibration

The chip ships uncalibrated; raw readings need scaling constants for voltage,
current, and power. Single-point calibration against a known load is the only
honest method, so the Calibration tab walks through it:

1. Plug a load of known, stable draw into the outlet (a meter reading plus a
   resistive load such as an incandescent bulb or a space heater on a fixed
   setting).
2. Enter the measured true watts (and volts/amps if measured).
3. The firmware computes the scaling constant from raw versus measured, stores it
   in config, and applies it going forward.

Firmware ships with Tasmota-style default constants so readings are in the right
ballpark before calibration.

## Reliability

- OTA over the network through the System tab; 4MB supports it comfortably.
- Wifi reconnect with backoff; fall back to setup AP after 30s of failure.
- MQTT reconnect with backoff and last-will for availability.
- Hardware watchdog kept fed; async loop avoids long blocking calls.

## Pending decisions

- Fallback-to-AP timeout fixed at 30s. Change if too aggressive.
- Indicator brightness default ~30%. Tune after seeing it on the wall.
- MQTT discovery prefix default `homeassistant`. Configurable on the MQTT tab.

## Companion Home Assistant integration

Built and published at [FireLabsCA/firelabs-hass](https://github.com/FireLabsCA/firelabs-hass):
a local-polling HACS integration that drives HA entities over the device HTTP API,
for setups that don't run MQTT. It also provides zeroconf auto-discovery via the
`_firelabs._tcp` mDNS service the firmware advertises. Use MQTT Discovery or the
integration, not both, to avoid duplicate entities.
