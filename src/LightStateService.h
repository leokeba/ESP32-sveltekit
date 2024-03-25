#ifndef LightStateService_h
#define LightStateService_h

/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2023 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#if FT_MQTT
#include <LightMqttSettingsService.h>
#include <MqttPubSub.h>
#endif

#include <LightArtNetSettingsService.h>

#include <HttpEndpoint.h>
#include <WebSocketServer.h>
// #include <WebSocketClient.h>
#include <ArtNetPubSub.h>

#define DEFAULT_LED_STATE false
#define OFF_STATE "OFF"
#define ON_STATE "ON"
#define DEFAULT_BRIGHTNESS 128

#define LIGHT_SETTINGS_ENDPOINT_PATH "/rest/lightState"
#define LIGHT_SETTINGS_SOCKET_PATH "/ws/lightState"

class LightState
{
public:
    bool ledOn;
    int brightness;

    static void read(LightState &settings, JsonObject &root)
    {
        root["led_on"] = settings.ledOn;
        root["brightness"] = settings.brightness;
    }

    static StateUpdateResult update(JsonObject &root, LightState &lightState)
    {   
        boolean newState = root["led_on"] | DEFAULT_LED_STATE;
        int newBrightness = root.containsKey("brightness") ? root["brightness"] : lightState.brightness;
        if ((lightState.ledOn != newState) || abs(lightState.brightness-newBrightness) > 0)
        {
            lightState.ledOn = newState;
            lightState.brightness = newBrightness;
            return StateUpdateResult::CHANGED;
        }
        return StateUpdateResult::UNCHANGED;
    }

    static void homeAssistRead(LightState &settings, JsonObject &root)
    {
        root["state"] = settings.ledOn ? ON_STATE : OFF_STATE;
        root["brightness"] = settings.brightness;
    }

    static StateUpdateResult homeAssistUpdate(JsonObject &root, LightState &lightState)
    {
        String state = root["state"];
        int newBrightness = root.containsKey("brightness") ? root["brightness"] : lightState.brightness;
        // parse new led state
        boolean newState = false;
        if (state.equals(ON_STATE))
        {
            newState = true;
        }
        else if (!state.equals(OFF_STATE))
        {
            return StateUpdateResult::ERROR;
        }
        // change the new state, if required
        if ((lightState.ledOn != newState) || (lightState.brightness != newBrightness))
        {
            lightState.ledOn = newState;
            lightState.brightness = newBrightness;
            return StateUpdateResult::CHANGED;
        }
        return StateUpdateResult::UNCHANGED;
    }

    static const uint16_t dmxChannels = 2;

    static void dmxRead(DmxFrame &data, JsonObject &root) {
        root["led_on"] = data.data[0] > 127;
        root["brightness"] = data.data[1];
    }
};

class LightStateService : public StatefulService<LightState>
{
public:
    LightStateService(PsychicHttpServer *server,
                      SecurityManager *securityManager,
#if FT_MQTT
                      PsychicMqttClient *mqttClient,
                      LightMqttSettingsService *lightMqttSettingsService,
#endif
                      LightArtNetSettingsService *lightArtNetSettingsService,
                      ArtnetWiFiReceiver *artNetReceiver);
    void begin();

private:
    HttpEndpoint<LightState> _httpEndpoint;
    WebSocketServer<LightState> _webSocketServer;
    //  WebSocketClient<LightState> _webSocketClient;
    ArtNetPubSub<LightState> _artNetPubSub;
#if FT_MQTT
    MqttPubSub<LightState> _mqttPubSub;
    PsychicMqttClient *_mqttClient;
    LightMqttSettingsService *_lightMqttSettingsService;
#endif
    LightArtNetSettingsService *_lightArtNetSettingsService;

    void registerConfig();
    void configureArtNet();
    void onConfigUpdated();
};

#endif
