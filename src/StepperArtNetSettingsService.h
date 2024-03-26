#ifndef StepperArtNetSettingsService_h
#define StepperArtNetSettingsService_h

#include <HttpEndpoint.h>
#include <FSPersistence.h>
#include <SettingValue.h>

#define STEPPER_ARTNET_SETTINGS_FILE "/config/stepperArtNetSettings.json"
#define STEPPER_ARTNET_SETTINGS_PATH "/rest/stepperArtNetSettings"

class StepperArtNetSettings
{
public:
    uint16_t channel = 1;
    uint16_t universe = 1;

    static void read(StepperArtNetSettings &settings, JsonObject &root)
    {
        root["channel"] = settings.channel;
        root["universe"] = settings.universe;
    }

    static StateUpdateResult update(JsonObject &root, StepperArtNetSettings &settings)
    {
        settings.channel = root["channel"];
        settings.universe = root["universe"];
        return StateUpdateResult::CHANGED;
    }
};

class StepperArtNetSettingsService : public StatefulService<StepperArtNetSettings>
{
public:
    StepperArtNetSettingsService(PsychicHttpServer *server, FS *fs, SecurityManager *securityManager);
    void begin();
    uint16_t getAddress();
    uint16_t getUniverse();

private:
    HttpEndpoint<StepperArtNetSettings> _httpEndpoint;
    FSPersistence<StepperArtNetSettings> _fsPersistence;
};

#endif // end StepperArtNetSettingsService_h
