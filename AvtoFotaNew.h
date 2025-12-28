#ifndef AvtoFotaNew_H
#define AvtoFotaNew_H

#include <Arduino.h>
#include <functional>

class AvtoFotaNew {
public:
    AvtoFotaNew(const char* version);

    // настройки
    void setManifestURL(const char* url);
    void setDebug(bool enable);
    void setProgressCallback(std::function<void(uint8_t)> cb);

    // информация
    bool getchekupdate();
    bool getupdate(String &ver, String &notes);
    uint8_t getpercentupdate() const;
    String getVER() const;

    // OTA
    bool updateNOW(bool autorestart = true); // блокирующий
    bool update(bool autorestart = true);    // неблокирующий (вызывать в loop)

private:
    enum OTAState {
        OTA_IDLE,
        OTA_BEGIN,
        OTA_DOWNLOAD,
        OTA_FINISH,
        OTA_DONE,
        OTA_ERROR
    };

    void feedWDT();
    void callProgress();

    // данные
    String _currentVersion;
    String _newVersion;
    String _notes;
    String _manifestURL;
    String _binURL;

    // состояние OTA
    OTAState _state;
    bool _updateAvailable;
    bool _autoRestart;
    bool _debug;

    uint32_t _totalSize;
    uint32_t _written;
    uint8_t  _progress;

    std::function<void(uint8_t)> _progressCb;

    // HTTP
    void* _http;
    void* _client;
};

#endif
