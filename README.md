# ESP8266-BME280-MQTT-SENSOR — publish BME280 temperature, humidity and pressure over MQTT

IoT example by [Fluidwire](https://fluidwire.com/).

Reads temperature, relative humidity and barometric pressure from a BME280 every 30 seconds and publishes them as JSON to an MQTT broker. A compact weather-station node on the cheap ESP8266 (NodeMCU / Wemos D1 mini) — a good fit for low-cost thesis and field deployments.

## What it does

- Connects to WiFi using credentials in `config.h`
- Connects to an MQTT broker (anonymous or username/password)
- Every 30 s, reads the BME280 and publishes JSON: `{"temp_c":27.4,"humidity":64.2,"pressure_hpa":1008.6,"uptime_s":120}`
- Auto-reconnects on WiFi or MQTT loss
- Auto-detects the BME280 at I2C address 0x76 or 0x77

## Required hardware

- 1x ESP8266 board (NodeMCU 1.0, Wemos D1 mini, or any ESP8266 with WiFi)
- 1x BME280 breakout (3.3 V I2C version)
- Jumper wires, breadboard

## Required libraries

Install via the Arduino IDE Library Manager (**Sketch → Include Library → Manage Libraries…**):

- **Adafruit BME280 Library** by Adafruit
- **Adafruit Unified Sensor** by Adafruit
- **PubSubClient** by Nick O'Leary

You also need the ESP8266 core installed via **Boards Manager**: add `http://arduino.esp8266.com/stable/package_esp8266com_index.json` to *Preferences → Additional Boards Manager URLs*, then search for `esp8266` by ESP8266 Community.

## Wiring

| BME280 pin | ESP8266 pin     |
|------------|-----------------|
| VIN / VCC  | 3V3             |
| GND        | GND             |
| SDA        | GPIO4 / D2      |
| SCL        | GPIO5 / D1      |

On NodeMCU / Wemos D1 mini the labels `D2` and `D1` map to `GPIO4` and `GPIO5`; the sketch uses the GPIO numbers. Don't confuse the BME280 with the BMP280 — the BMP280 has no humidity sensor and will read humidity as 0 or fail the chip-ID check.

## Setup

1. Copy `config.h.example` to `config.h`.
2. Edit `config.h` with your WiFi SSID/password and MQTT broker details. The defaults point at the public HiveMQ broker (anonymous, port 1883) — fine for first-light testing.
3. In the Arduino IDE, select **Tools → Board → NodeMCU 1.0 (ESP-12E Module)** (or your specific ESP8266 board) and the correct COM port.
4. Click **Upload**.

## Expected output

Open **Tools → Serial Monitor**, baud rate **115200**:

```
WiFi: connecting to your-wifi-ssid .... OK, ip=192.168.1.42
MQTT: connecting to broker.hivemq.com:1883 as fluidwire-esp8266-bme280 ... OK
MQTT pub fluidwire/demo/bme280 -> {"temp_c":27.4,"humidity":64.2,"pressure_hpa":1008.6,"uptime_s":30}
```

Subscribe to verify:

```
mosquitto_sub -h broker.hivemq.com -t 'fluidwire/demo/bme280' -v
```

## Troubleshooting

- **`BME280 not found at 0x76/0x77`** — SDA/SCL swapped or loose, sensor not powered, or it's a BMP280. Run an I2C scanner to confirm the address.
- **Temperature reads a few °C high** — self-heating from the board or radio. Keep the sensor away from the ESP8266 module.
- **`MQTT: connecting ... FAIL rc=-2`** — broker host unreachable. Check WiFi got an IP and the broker hostname.
- **`MQTT: connecting ... FAIL rc=5`** — auth refused. Check the username/password.
- **Connects, publishes, but nothing on the subscriber side** — topic mismatch. Topics are case-sensitive.
- **WiFi connect spins forever** — wrong `WIFI_SSID`, or a 5 GHz-only network. The ESP8266 is 2.4 GHz only.

## Security note

Don't commit `config.h`. The `.gitignore` in this repo already excludes it; only `config.h.example` is tracked. For production use, prefer TLS (`WiFiClientSecure` + port 8883) and per-device credentials.

## License

MIT — see [LICENSE](LICENSE).

## Got stuck?

Drop by [fluidwire.com](https://fluidwire.com/) — we ship IoT building blocks and are happy to help.
