#ifndef LightArtNetSettingsService_h
#define LightArtNetSettingsService_h

#include <HttpEndpoint.h>
#include <FSPersistence.h>
#include <SettingValue.h>

#define LIGHT_ARTNET_SETTINGS_FILE "/config/lightArtNetSettings.json"
#define LIGHT_ARTNET_SETTINGS_PATH "/rest/lightArtNetSettings"

class LightArtNetSettings
{
public:
    uint16_t channel = 1;
    uint16_t universe = 1;

    static void read(LightArtNetSettings &settings, JsonObject &root)
    {
        root["channel"] = settings.channel;
        root["universe"] = settings.universe;
    }

    static StateUpdateResult update(JsonObject &root, LightArtNetSettings &settings)
    {
        settings.channel = root["channel"];
        settings.universe = root["universe"];
        return StateUpdateResult::CHANGED;
    }
};

class LightArtNetSettingsService : public StatefulService<LightArtNetSettings>
{
public:
    LightArtNetSettingsService(PsychicHttpServer *server, FS *fs, SecurityManager *securityManager);
    void begin();
    uint16_t getAddress();
    uint16_t getUniverse();

private:
    HttpEndpoint<LightArtNetSettings> _httpEndpoint;
    FSPersistence<LightArtNetSettings> _fsPersistence;
};

#endif // end LightArtNetSettingsService_h
