#include "AvtoFotaNew.h"
#include <ArduinoJson.h>
#include <Update.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include <WiFiClientSecureBearSSL.h>
#else
  #include <WiFi.h>
  #include <HTTPClient.h>
  #include <WiFiClientSecure.h>
#endif

#define OTA_CHUNK 2048

AvtoFotaNew::AvtoFotaNew(const char* version)
: _currentVersion(version),
  _state(OTA_IDLE),
  _updateAvailable(false),
  _autoRestart(true),
  _debug(true),
  _totalSize(0),
  _written(0),
  _progress(0),
  _http(nullptr),
  _client(nullptr) {}

void AvtoFotaNew::setManifestURL(const char* url) {
    _manifestURL = url;
}

void AvtoFotaNew::setDebug(bool enable) {
    _debug = enable;
}

void AvtoFotaNew::setProgressCallback(std::function<void(uint8_t)> cb) {
    _progressCb = cb;
}

void AvtoFotaNew::feedWDT() {
    yield();
#if defined(ESP8266)
    ESP.wdtFeed();
#endif
}

void AvtoFotaNew::callProgress() {
    if (_progressCb) _progressCb(_progress);
}

bool AvtoFotaNew::getchekupdate() {
    if (WiFi.status() != WL_CONNECTED) return false;

#if defined(ESP8266)
    BearSSL::WiFiClientSecure client;
    client.setInsecure();
#else
    WiFiClientSecure client;
    client.setInsecure();
#endif

    HTTPClient http;
    if (!http.begin(client, _manifestURL)) return false;

    if (http.GET() != HTTP_CODE_OK) {
        http.end();
        return false;
    }

    DynamicJsonDocument doc(2048);
    deserializeJson(doc, http.getString());
    http.end();

    _newVersion = doc["version"].as<String>();
    _notes      = doc["notes"].as<String>();
    _binURL     = doc["bin"].as<String>();

    _updateAvailable = (_newVersion != _currentVersion);
    return _updateAvailable;
}

bool AvtoFotaNew::getupdate(String &ver, String &notes) {
    if (!_updateAvailable) return false;
    ver = _newVersion;
    notes = _notes;
    return true;
}

uint8_t AvtoFotaNew::getpercentupdate() const {
    return _progress;
}

String AvtoFotaNew::getVER() const {
    return _currentVersion;
}

/* ================= ¡ÀŒ »–”ﬁŸ»… OTA ================= */

bool AvtoFotaNew::updateNOW(bool autorestart) {
    if (!_updateAvailable) return false;

    _autoRestart = autorestart;
    _progress = 0;
    _written = 0;

#if defined(ESP8266)
    BearSSL::WiFiClientSecure client;
    client.setInsecure();
#else
    WiFiClientSecure client;
    client.setInsecure();
#endif

    HTTPClient http;
    http.setTimeout(300000);
    if (!http.begin(client, _binURL)) return false;

    if (http.GET() != HTTP_CODE_OK) {
        http.end();
        return false;
    }

    _totalSize = http.getSize();
    if (_totalSize <= 0) return false;
    if (!Update.begin(_totalSize)) return false;

    WiFiClient* s = http.getStreamPtr();
    uint8_t buf[OTA_CHUNK];

    while (http.connected() && _written < _totalSize) {
        int len = s->readBytes(buf, OTA_CHUNK);
        if (len > 0) {
            Update.write(buf, len);
            _written += len;
            _progress = (_written * 100) / _totalSize;
            callProgress();
        }
        feedWDT();
    }

    http.end();

    if (Update.end(true)) {
        _currentVersion = _newVersion;
        if (_autoRestart) ESP.restart();
        return true;
    }

    return false;
}

/* ================= Õ≈¡ÀŒ »–”ﬁŸ»… OTA ================= */

bool AvtoFotaNew::update(bool autorestart) {
    feedWDT();

    switch (_state) {

    case OTA_IDLE:
        if (!_updateAvailable) return false;
        _autoRestart = autorestart;
        _written = 0;
        _progress = 0;

#if defined(ESP8266)
        _client = new BearSSL::WiFiClientSecure();
        ((BearSSL::WiFiClientSecure*)_client)->setInsecure();
#else
        _client = new WiFiClientSecure();
        ((WiFiClientSecure*)_client)->setInsecure();
#endif

        _http = new HTTPClient();
        ((HTTPClient*)_http)->begin(*(WiFiClientSecure*)_client, _binURL);
        ((HTTPClient*)_http)->GET();

        _totalSize = ((HTTPClient*)_http)->getSize();
        Update.begin(_totalSize);

        _state = OTA_DOWNLOAD;
        break;

    case OTA_DOWNLOAD: {
        WiFiClient* s = ((HTTPClient*)_http)->getStreamPtr();
        uint8_t buf[OTA_CHUNK];
        int avail = s->available();
        if (avail > 0) {
            int r = s->readBytes(buf, min(avail, OTA_CHUNK));
            Update.write(buf, r);
            _written += r;
            _progress = (_written * 100) / _totalSize;
            callProgress();
        }
        if (_written >= _totalSize) _state = OTA_FINISH;
        break;
    }

    case OTA_FINISH:
        ((HTTPClient*)_http)->end();
        delete (HTTPClient*)_http;
        delete (WiFiClientSecure*)_client;
        _http = nullptr;
        _client = nullptr;

        if (Update.end(true)) {
            _currentVersion = _newVersion;
            _state = OTA_DONE;
            if (_autoRestart) ESP.restart();
            return true;
        }
        _state = OTA_ERROR;
        break;

    case OTA_DONE:
    case OTA_ERROR:
        return true;
    }

    return false;
}
