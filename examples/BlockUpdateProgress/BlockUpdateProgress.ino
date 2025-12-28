#include <Arduino.h>
#include <ArduinoJson.h>
#include <AvtoFotaNew.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#else
  #include <WiFi.h>
#endif

const char* WIFI_SSID = "Balk";
const char* WIFI_PASS = "13051973";
const char* MANIFEST_URL = "https://raw.githubusercontent.com/eu1abg/Webasto_virtuino/main/firmware/firmware.json";

AvtoFotaNew fota("1.0.0");

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  fota.setManifestURL(MANIFEST_URL);

  // 🔥 ВОТ КЛЮЧЕВОЕ
  fota.setProgressCallback([](uint8_t p) {
    Serial.printf("OTA progress: %d%%\n", p);
  });

  if (!fota.getchekupdate()) {
    Serial.println("No update");
    return;
  }

  Serial.println("Starting BLOCKING OTA...");
  fota.updateNOW(true);
}

void loop() {}
