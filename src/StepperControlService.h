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
        bool hasChanged = false;
        if (root["isEnabled"].is<bool>() && settings.isEnabled != root["isEnabled"]) {
            settings.isEnabled = root["isEnabled"];
            hasChanged = true;
        }
        if (root["direction"].is<bool>() && settings.direction != root["direction"]) {
            settings.direction = root["direction"];
            hasChanged = true;
        }
        if (root["move"].is<float>() && abs(settings.move - float(root["move"])) > 0.01) {
            settings.newMove = settings.move - float(root["move"]);
            settings.move = root["move"];
            hasChanged = true;
        }
        if (root["speed"].is<float>() && abs(settings.speed - float(root["speed"])) > 0.01) {
            settings.speed = root["speed"];
            hasChanged = true;
        }
        if (root["acceleration"].is<float>() &&  abs(settings.acceleration - float(root["acceleration"])) > 0.01) {
            settings.acceleration = root["acceleration"];
            hasChanged = true;
        }
        // Serial.print("Update : ");
        // Serial.println(float(root["speed"]));
        if (hasChanged) return StateUpdateResult::CHANGED;
        else return StateUpdateResult::UNCHANGED;
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
