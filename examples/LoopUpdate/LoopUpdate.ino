#include <Arduino.h>
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

bool otaStarted = false;

/* ================= SETUP ================= */

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== NON-BLOCKING OTA EXAMPLE ===");

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

  // Проверяем обновление один раз
  if (fota.getchekupdate()) {
    String ver, notes;
    fota.getupdate(ver, notes);

    Serial.println("=== UPDATE AVAILABLE ===");
    Serial.print("New version : "); Serial.println(ver);
    Serial.print("Notes       : "); Serial.println(notes);
    otaStarted = true;
  } else {
    Serial.println("No updates available");
  }
}

/* ================= LOOP ================= */

void loop() {

  if (otaStarted) {
    // Неблокирующий OTA
    bool finished = fota.update(true);

    Serial.print("OTA progress: ");
    Serial.print(fota.getpercentupdate());
    Serial.println(" %");

    if (finished) {
      Serial.println("OTA finished (device may reboot)");
      otaStarted = false;
    }
  }

  // Здесь ПАРАЛЛЕЛЬНО может работать остальная логика
  static uint32_t t = 0;
  if (millis() - t > 2000) {
    t = millis();
    Serial.println("Main loop still alive...");
  }
}
