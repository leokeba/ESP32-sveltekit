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
    float move;
    float newMove;
    float speed;
    float acceleration;
    // StepperSettings settings;

    static void read(StepperControl &settings, JsonObject &root)
    {
        root["isEnabled"] = settings.isEnabled;
        root["direction"] = settings.direction;
        root["move"] = settings.move;
        root["speed"] = settings.speed;
        root["acceleration"] = settings.acceleration;
        // Serial.print("Read : ");
        // Serial.println(float(root["speed"]));
    }

    static StateUpdateResult update(JsonObject &root, StepperControl &settings)
    {   
        if (root["isEnabled"].is<bool>()) settings.isEnabled = root["isEnabled"];
        if (root["direction"].is<bool>()) settings.direction = root["direction"];
        if (root["move"].is<float>()) {
            settings.newMove = settings.move - float(root["move"]);
            settings.move = root["move"];
        }
        if (root["speed"].is<float>()) settings.speed = root["speed"];
        if (root["acceleration"].is<float>()) settings.acceleration = root["acceleration"];
        // Serial.print("Update : ");
        // Serial.println(float(root["speed"]));
        return StateUpdateResult::CHANGED;
    }

    static const uint16_t dmxChannels = 5;

    static void dmxRead(DmxFrame &data, JsonObject &root) {
        root["isEnabled"] = data.data[0] > 127;
        root["direction"] = data.data[1] > 127;
        root["speed"] = data.data[2] / 255.;
        // Serial.print("Read DMX : ");
        // Serial.println(float(root["speed"]));
        root["move"] = data.data[3] / 255.;
        root["acceleration"] = data.data[4] / 255.;
    }

    static void readState(TMC5160Controller *stepper, StepperSettingsService *stepperSettingsService, JsonObject &root) {
        root["isEnabled"] = stepper->enabled;
        root["direction"] = stepper->getSpeed() >= 0;
        // root["move"] = stepper->move();
        root["speed"] = abs(stepper->getSpeed());
        // Serial.print("Read State : ");
        // Serial.println(float(root["speed"]));
        root["acceleration"] = stepper->getAcceleration();
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
