# FireLabs S31 firmware

Custom firmware for the Sonoff S31 smart plug (ESP8266, 4MB). It replaces whatever firmware is on the plug (stock Sonoff, or ESPHome if you flashed that yourself) with an MQTT-native firmware that talks to Home Assistant through MQTT Discovery, plus a local HTTP API and a web UI for setup and configuration. It also works on the S31 Lite (no power-metering chip), detecting the missing CSE7766 at runtime and hiding the power features.

A companion Home Assistant integration (no MQTT required) lives at [FireLabsCA/firelabs-hass](https://github.com/FireLabsCA/firelabs-hass). Once you've flashed a plug, install that integration and add it:

[![Open your Home Assistant instance and open the FireLabs repository inside HACS.](https://my.home-assistant.io/badges/hacs_repository.svg)](https://my.home-assistant.io/redirect/hacs_repository/?owner=FireLabsCA&repository=firelabs-hass&category=integration)
[![Open your Home Assistant instance and start setting up FireLabs.](https://my.home-assistant.io/badges/config_flow_start.svg)](https://my.home-assistant.io/redirect/config_flow_start/?domain=firelabs)

## Features

- Relay control; the GPIO0 button toggles it, and a 5-second hold factory-resets.
- CSE7766 power metering: voltage, current, watts, daily energy.
- WiFi captive-portal setup wizard (name + network), with fallback to the setup AP if it can't connect.
- Web UI: status with live power/voltage graphs, MQTT config, settings, calibration, OTA.
- MQTT Discovery into Home Assistant, with last-will availability.
- mDNS `_firelabs._tcp` advertisement for Home Assistant zeroconf discovery.
- Over-the-air updates after the first flash.

See [SPEC.md](SPEC.md) for the full design.

## Build

PlatformIO:

```
pio run -e s31
```

The binary lands at `.pio/build/s31/firmware.bin`. Each release on GitHub also ships a prebuilt `firmware.bin`.

There is a second environment, `s31_diag`, that disables the meter and turns UART0 into a 115200 debug console for troubleshooting.

## Flash

The first flash has to go over serial; neither the stock Sonoff firmware nor ESPHome will accept this image through its own OTA:

```
pio run -e s31 -t upload --upload-port /dev/cu.usbserial-XXXX
```

Unplug from mains, wire a 3.3V USB-TTL adapter to the header, and hold the button while applying power to enter flash mode. After this first flash, every later update is over the air, from the web UI System tab or:

```
curl -F "firmware=@.pio/build/s31/firmware.bin" http://<plug-ip>/update
```

The same `firmware.bin` works on every plug; identity comes from the MAC at runtime, so each one self-names.

## First boot

With no saved wifi, the plug starts an open AP named `FireLabs S31 <mac-suffix>`. Connect to it, and a captive portal walks through picking your network and naming the plug.

## Support

Questions, updates, and works in progress: [FireBall Codes on Discord](https://discord.gg/QpV82CFfVD).

If this saved you some time, you can [buy me a sushi roll](https://ko-fi.com/fireball1725).

## License

[GNU AGPL-3.0](LICENSE).
