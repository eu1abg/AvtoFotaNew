#include <Arduino.h>
#include <ArduinoJson.h>
#include <AvtoFotaNew.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#else
  #include <WiFi.h>
#endif

/* ================= НАСТРОЙКИ ================= */

const char* WIFI_SSID = "Balk";
const char* WIFI_PASS = "13051973";

const char* MANIFEST_URL ="https://raw.githubusercontent.com/eu1abg/Webasto_virtuino/main/firmware/firmware.json";

/* ================= FOTA ================= */

AvtoFotaNew fota("1.0.0");

/* ================= SETUP ================= */

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== BLOCKING OTA EXAMPLE ===");

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("WiFi connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  fota.setManifestURL(MANIFEST_URL);
  fota.setDebug(true);

  if (!fota.getchekupdate()) {
    Serial.println("No updates available");
    return;
  }

  String ver, notes;
  fota.getupdate(ver, notes);

  Serial.println("=== UPDATE AVAILABLE ===");
  Serial.print("New version : "); Serial.println(ver);
  Serial.print("Notes       : "); Serial.println(notes);

  Serial.println("Starting BLOCKING OTA...");
  Serial.println("Device will freeze until OTA ends!");

  bool ok = fota.updateNOW(true);

  if (ok) Serial.println("OTA SUCCESS");
  else    Serial.println("OTA FAILED");
}

void loop() {
  // В блокирующем режиме loop почти не используется
}
