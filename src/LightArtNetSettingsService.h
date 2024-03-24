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
    uint16_t address = 1;

    static void read(LightArtNetSettings &settings, JsonObject &root)
    {
        root["address"] = settings.address;
    }

    static StateUpdateResult update(JsonObject &root, LightArtNetSettings &settings)
    {
        settings.address = root["address"];
        return StateUpdateResult::CHANGED;
    }
};

class LightArtNetSettingsService : public StatefulService<LightArtNetSettings>
{
public:
    LightArtNetSettingsService(PsychicHttpServer *server, FS *fs, SecurityManager *securityManager);
    void begin();
    uint16_t getAddress();

private:
    HttpEndpoint<LightArtNetSettings> _httpEndpoint;
    FSPersistence<LightArtNetSettings> _fsPersistence;
};

#endif // end LightArtNetSettingsService_h
