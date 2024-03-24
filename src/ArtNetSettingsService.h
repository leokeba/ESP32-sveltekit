#ifndef ArtNetSettingsService_h
#define ArtNetSettingsService_h

#include <StatefulService.h>
#include <HttpEndpoint.h>
#include <FSPersistence.h>
#include <SettingValue.h>
#include <WiFi.h>
#include <ArtnetWiFi.h>

#ifndef FACTORY_ARTNET_ENABLED
#define FACTORY_ARTNET_ENABLED false
#endif

#ifndef FACTORY_ARTNET_PORT
#define FACTORY_ARTNET_PORT 6454
#endif

#define ARTNET_SETTINGS_FILE "/config/artNetSettings.json"
#define ARTNET_SETTINGS_SERVICE_PATH "/rest/artNetSettings"

class ArtNetSettings
{
public:
    bool enabled = FACTORY_ARTNET_ENABLED;
    int port = FACTORY_ARTNET_PORT;

    static void read(ArtNetSettings &settings, JsonObject &root)
    {
        root["enabled"] = settings.enabled;
        root["port"] = settings.port;
    }

    static StateUpdateResult update(JsonObject &root, ArtNetSettings &settings)
    {
        settings.enabled = root["enabled"] | FACTORY_ARTNET_ENABLED;
        settings.port = root["port"] | FACTORY_ARTNET_PORT;
        return StateUpdateResult::CHANGED;
    }
};

class ArtNetSettingsService : public StatefulService<ArtNetSettings>
{
public:
    ArtNetSettingsService(PsychicHttpServer *server, FS *fs, SecurityManager *securityManager, ArtnetWiFiReceiver *artNetReceiver);
    ~ArtNetSettingsService();

    void begin();
    void loop();
    bool isEnabled();
    int getPort();

protected:
    void onConfigUpdated();

private:
    PsychicHttpServer *_server;
    SecurityManager *_securityManager;
    ArtnetWiFiReceiver *_artNetReceiver;
    HttpEndpoint<ArtNetSettings> _httpEndpoint;
    FSPersistence<ArtNetSettings> _fsPersistence;

    bool _hasBegun = false;

    void onStationModeGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
    void onStationModeDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);

};

#endif // end ArtNetSettingsService_h
