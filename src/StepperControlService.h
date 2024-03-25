#ifndef StepperControlService_h
#define StepperControlService_h

#include <LightArtNetSettingsService.h>

#include <HttpEndpoint.h>
#include <WebSocketServer.h>
#include <ArtNetPubSub.h>
#include <tmcdriver.h>

#define DEFAULT_LED_STATE false
#define OFF_STATE "OFF"
#define ON_STATE "ON"
#define DEFAULT_BRIGHTNESS 128

#define STEPPER_CONTROL_ENDPOINT_PATH "/rest/stepperControl"
#define STEPPER_CONTROL_SOCKET_PATH "/ws/stepperControl"

class StepperControl
{
public:
    bool isEnabled;
    bool direction;
    int32_t move;
    int32_t speed;
    uint32_t acceleration;
    uint32_t current;

    static void read(StepperControl &settings, JsonObject &root)
    {
        root["isEnabled"] = settings.isEnabled;
        root["direction"] = settings.direction;
        root["move"] = settings.move;
        root["speed"] = settings.speed;
        root["acceleration"] = settings.acceleration;
        root["current"] = settings.current;
    }

    static StateUpdateResult update(JsonObject &root, StepperControl &settings)
    {   
        settings.isEnabled = root["isEnabled"];
        settings.direction = root["direction"];
        settings.move = root["move"];
        settings.speed = root["speed"];
        settings.acceleration = root["acceleration"];
        settings.current = root["current"];
        return StateUpdateResult::CHANGED;
    }

    static const uint16_t dmxChannels = 2;

    static void dmxRead(DmxFrame &data, JsonObject &root) {
        // root["led_on"] = data.data[0] > 127;
        // root["brightness"] = data.data[1];
    }

    static void readState(TMC5160Controller *stepper, JsonObject &root) {
        root["isEnabled"] = stepper->enabled;
        root["direction"] = stepper->direction;
        root["move"] = stepper->move();
        root["speed"] = stepper->getSpeed();
        root["acceleration"] = stepper->getAcceleration();
        root["current"] = stepper->driver.rms_current();
    }
};

class StepperControlService : public StatefulService<StepperControl>
{
public:
    StepperControlService(PsychicHttpServer *server,
                      SecurityManager *securityManager,
                      LightArtNetSettingsService *lightArtNetSettingsService,
                      ArtnetWiFiReceiver *artNetReceiver,
                      TMC5160Controller *stepper);
    void begin();

private:
    HttpEndpoint<StepperControl> _httpEndpoint;
    WebSocketServer<StepperControl> _webSocketServer;
    //  WebSocketClient<StepperControl> _webSocketClient;
    ArtNetPubSub<StepperControl> _artNetPubSub;
    LightArtNetSettingsService *_lightArtNetSettingsService;
    TMC5160Controller *_stepper;

    void registerConfig();
    void configureArtNet();
    void onConfigUpdated();
    void updateState();
};

#endif
