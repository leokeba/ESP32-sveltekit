#ifndef StepperControlService_h
#define StepperControlService_h

#include <StepperArtNetSettingsService.h>
#include <StepperSettingsService.h>

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
    int32_t newMove;
    int32_t speed;
    uint32_t acceleration;

    static void read(StepperControl &settings, JsonObject &root)
    {
        root["isEnabled"] = settings.isEnabled;
        root["direction"] = settings.direction;
        root["move"] = settings.move;
        root["speed"] = settings.speed;
        root["acceleration"] = settings.acceleration;
    }

    static StateUpdateResult update(JsonObject &root, StepperControl &settings)
    {   
        settings.isEnabled = root["isEnabled"];
        settings.direction = root["direction"];
        settings.newMove = settings.move - int32_t(root["move"]);
        settings.move = root["move"];
        settings.speed = root["speed"];
        settings.acceleration = root["acceleration"];
        return StateUpdateResult::CHANGED;
    }

    static const uint16_t dmxChannels = 5;

    static void dmxRead(DmxFrame &data, JsonObject &root) {
        root["isEnabled"] = data.data[0] > 127;
        root["direction"] = data.data[1] > 127;
        root["speed"] = data.data[2];
        root["move"] = data.data[3];
        root["acceleration"] = data.data[4];
    }

    static void readState(TMC5160Controller *stepper, StepperSettingsService *stepperSettingsService, JsonObject &root) {
        root["isEnabled"] = stepper->enabled;
        root["direction"] = stepper->getSpeed() >= 0;
        // root["move"] = stepper->move();
        root["speed"] = abs(stepper->getSpeed() * 1024 / stepperSettingsService->getMaxSpeed());
        root["acceleration"] = stepper->getAcceleration() * 1024 / stepperSettingsService->getMaxAccel();
    }
};

class StepperControlService : public StatefulService<StepperControl>
{
public:
    StepperControlService(PsychicHttpServer *server,
                      SecurityManager *securityManager,
                      StepperArtNetSettingsService *stepperArtNetSettingsService,
                      StepperSettingsService *stepperSettingsService,
                      ArtnetWiFiReceiver *artNetReceiver,
                      TMC5160Controller *stepper);
    void begin();
    void loop();

private:
    HttpEndpoint<StepperControl> _httpEndpoint;
    WebSocketServer<StepperControl> _webSocketServer;
    //  WebSocketClient<StepperControl> _webSocketClient;
    ArtNetPubSub<StepperControl> _artNetPubSub;
    StepperArtNetSettingsService *_stepperArtNetSettingsService;
    StepperSettingsService *_stepperSettingsService;
    TMC5160Controller *_stepper;

    unsigned long lastUpdate = 0;

    void registerConfig();
    void configureArtNet();
    void onConfigUpdated(const String &originId);
    void updateState();
};

#endif
