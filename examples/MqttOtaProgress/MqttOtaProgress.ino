#include <Arduino.h>
#include <ArduinoJson.h>
#include <AvtoFotaNew.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#else
  #include <WiFi.h>
#endif

#include <PubSubClient.h>

/* ================= НАСТРОЙКИ ================= */

// WiFi
const char* WIFI_SSID = "YOUR_WIFI";
const char* WIFI_PASS = "YOUR_PASS";

// MQTT
const char* MQTT_HOST = "m6.wqtt.ru";
const uint16_t MQTT_PORT = 14516;
const char* MQTT_USER = "eu1abg";
const char* MQTT_PASS = "13051973";

const char* MQTT_CLIENT_ID = "esp32_ota";

// Топики
const char* TOPIC_PROGRESS = "device/esp32/fota/progress";
const char* TOPIC_STATE    = "device/esp32/fota/state";
const char* TOPIC_VERSION  = "device/esp32/fota/version";

// OTA
const char* MANIFEST_URL =
"https://yourserver.com/firmware.json";

/* ================= ОБЪЕКТЫ ================= */

WiFiClient wifi;
PubSubClient mqtt(wifi);
AvtoFotaNew fota("1.0.0");

/* ================= MQTT ================= */

void mqttReconnect() {
  while (!mqtt.connected()) {
    if (mqtt.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS)) {
      mqtt.publish(TOPIC_STATE, "online", true);
    } else {
      delay(2000);
    }
  }
}

/* ================= SETUP ================= */

void setup() {
  Serial.begin(115200);
  delay(1000);

  /* ---------- WiFi ---------- */
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  /* ---------- MQTT ---------- */
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqttReconnect();

  /* ---------- OTA ---------- */
  fota.setManifestURL(MANIFEST_URL);

  // 🔥 ВОТ ГЛАВНОЕ — MQTT-прогресс
  fota.setProgressCallback([](uint8_t p) {
    char buf[4];
    snprintf(buf, sizeof(buf), "%d", p);
    mqtt.publish(TOPIC_PROGRESS, buf, true);

    if (p == 0)   mqtt.publish(TOPIC_STATE, "downloading", true);
    if (p == 100) mqtt.publish(TOPIC_STATE, "installing", true);
  });

  mqtt.publish(TOPIC_STATE, "checking", true);

  if (!fota.getchekupdate()) {
    mqtt.publish(TOPIC_STATE, "no_update", true);
    return;
  }

  mqtt.publish(TOPIC_STATE, "update_found", true);
  mqtt.publish(TOPIC_VERSION, fota.getVER().c_str(), true);

  /* ---------- START OTA ---------- */
  mqtt.publish(TOPIC_STATE, "start", true);
  fota.updateNOW(true);
}

/* ================= LOOP ================= */

void loop() {
  mqtt.loop();
}

