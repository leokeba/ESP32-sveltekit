#ifndef StepperSettingsService_h
#define StepperSettingsService_h

#include <StepperArtNetSettingsService.h>

#include <HttpEndpoint.h>
#include <FSPersistence.h>
#include <tmcdriver.h>

#define STEPPER_SETTINGS_FILE "/config/stepperSettings.json"
#define STEPPER_SETTINGS_ENDPOINT_PATH "/rest/stepperSettings"

class StepperSettings
{
public:
    bool enableOnStart;
    bool invertDirection;
    int32_t maxSpeed;
    uint32_t maxAcceleration;
    uint16_t current;

    static void read(StepperSettings &settings, JsonObject &root)
    {
        root["enableOnStart"] = settings.enableOnStart;
        root["invertDirection"] = settings.invertDirection;
        root["maxSpeed"] = settings.maxSpeed;
        root["maxAcceleration"] = settings.maxAcceleration;
        root["current"] = settings.current;
    }

    static StateUpdateResult update(JsonObject &root, StepperSettings &settings)
    {   
        settings.enableOnStart = root["enableOnStart"];
        settings.invertDirection = root["invertDirection"];
        settings.maxSpeed = root["maxSpeed"];
        settings.maxAcceleration = root["maxAcceleration"];
        settings.current = root["current"];
        return StateUpdateResult::CHANGED;
    }

    // static void readState(TMC5160Controller *stepper, JsonObject &root) {
    //     root["enableOnStart"] = stepper->enabled;
    //     root["invertDirection"] = stepper->invertDirection;
    //     // root["move"] = stepper->move();
    //     root["maxSpeed"] = stepper->getSpeed();
    //     root["maxAcceleration"] = stepper->getAcceleration();
    //     root["current"] = stepper->driver.rms_current();
    // }
};

class StepperSettingsService : public StatefulService<StepperSettings>
{
public:
    StepperSettingsService(PsychicHttpServer *server, 
                            FS *fs,
                            SecurityManager *securityManager,
                            TMC5160Controller *stepper);
    void begin();
    int32_t getMaxSpeed();
    int32_t getMaxAccel();

private:
    HttpEndpoint<StepperSettings> _httpEndpoint;
    FSPersistence<StepperSettings> _fsPersistence;
    TMC5160Controller *_stepper;

    void onConfigUpdated();
};

#endif
