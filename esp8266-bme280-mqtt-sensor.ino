/*
 * ESP8266 BME280 MQTT Weather Node
 * --------------------------------
 * Reads temperature, humidity and barometric pressure from a BME280 over
 * I2C every 30 seconds and publishes them as JSON to an MQTT broker.
 * A compact weather-station node for low-cost thesis deployments.
 *
 * Required libraries (install via Library Manager):
 *   - "Adafruit BME280 Library" by Adafruit
 *   - "Adafruit Unified Sensor" by Adafruit
 *   - "PubSubClient" by Nick O'Leary
 *
 * Board: any ESP8266 board (NodeMCU 1.0 / Wemos D1 mini)
 *
 * Wiring (I2C):
 *   BME280 VIN -> ESP8266 3V3
 *   BME280 GND -> ESP8266 GND
 *   BME280 SDA -> GPIO4 / D2
 *   BME280 SCL -> GPIO5 / D1
 *
 * Setup:
 *   1. Copy config.h.example to config.h
 *   2. Edit config.h with your WiFi creds + MQTT broker details
 *   3. Flash. Open Serial Monitor at 115200.
 *
 * Sample MQTT message:
 *   {"temp_c":27.4,"humidity":64.2,"pressure_hpa":1008.6,"uptime_s":120}
 *
 * (c) 2026 Fluidwire. Released under the MIT License.
 * https://github.com/fluidwire
 */

#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <PubSubClient.h>
#include "config.h"

#define I2C_SDA 4   // D2
#define I2C_SCL 5   // D1

const uint32_t PUBLISH_INTERVAL_MS = 30000;

WiFiClient     wifiClient;
PubSubClient   mqtt(wifiClient);
Adafruit_BME280 bme;

uint32_t lastPublish = 0;

void connectWiFi() {
  Serial.printf("WiFi: connecting to %s ", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf(" OK, ip=%s\n", WiFi.localIP().toString().c_str());
}

void connectMQTT() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  while (!mqtt.connected()) {
    Serial.printf("MQTT: connecting to %s:%u as %s ... ",
                  MQTT_HOST, MQTT_PORT, MQTT_CLIENT_ID);
    bool ok = strlen(MQTT_USER) > 0
      ? mqtt.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)
      : mqtt.connect(MQTT_CLIENT_ID);
    if (ok) {
      Serial.println("OK");
    } else {
      Serial.printf("FAIL rc=%d, retry in 5s\n", mqtt.state());
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Wire.begin(I2C_SDA, I2C_SCL);
  // Most breakouts use 0x76 (SDO to GND) or 0x77 (SDO to VCC).
  if (!bme.begin(0x76) && !bme.begin(0x77)) {
    Serial.println("BME280 not found at 0x76/0x77 - check wiring");
    while (true) delay(1000);
  }
  connectWiFi();
  connectMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  uint32_t now = millis();
  if (now - lastPublish < PUBLISH_INTERVAL_MS) return;
  lastPublish = now;

  float t = bme.readTemperature();
  float h = bme.readHumidity();
  float p = bme.readPressure() / 100.0f;  // Pa -> hPa
  if (isnan(t) || isnan(h) || isnan(p)) {
    Serial.println("BME280 read failed, skipping publish");
    return;
  }

  char payload[160];
  snprintf(payload, sizeof(payload),
           "{\"temp_c\":%.1f,\"humidity\":%.1f,\"pressure_hpa\":%.1f,\"uptime_s\":%lu}",
           t, h, p, (unsigned long)(now / 1000));

  if (mqtt.publish(MQTT_TOPIC, payload)) {
    Serial.printf("MQTT pub %s -> %s\n", MQTT_TOPIC, payload);
  } else {
    Serial.println("MQTT publish failed");
  }
}
