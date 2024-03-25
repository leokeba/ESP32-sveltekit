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

#include <LightStateService.h>

LightStateService::LightStateService(PsychicHttpServer *server,
                                     SecurityManager *securityManager,

#if FT_MQTT
                                     PsychicMqttClient *mqttClient,
                                     LightMqttSettingsService *lightMqttSettingsService,
#endif
                                     LightArtNetSettingsService *lightArtNetSettingsService,
                                     ArtnetWiFiReceiver *artNetReceiver) :                  
                                                                            _httpEndpoint(LightState::read,
                                                                                            LightState::update,
                                                                                            this,
                                                                                            server,
                                                                                            LIGHT_SETTINGS_ENDPOINT_PATH,
                                                                                            securityManager,
                                                                                            AuthenticationPredicates::IS_AUTHENTICATED),
                                                                            _webSocketServer(LightState::read,
                                                                                                LightState::update,
                                                                                                this,
                                                                                                server,
                                                                                                LIGHT_SETTINGS_SOCKET_PATH,
                                                                                                securityManager,
                                                                                                AuthenticationPredicates::IS_AUTHENTICATED),
#if FT_MQTT
                                                                            _mqttClient(mqttClient),
                                                                            _lightMqttSettingsService(lightMqttSettingsService),
                                                                            _mqttPubSub(LightState::homeAssistRead, LightState::homeAssistUpdate, this, mqttClient),
#endif
                                                                            _lightArtNetSettingsService(lightArtNetSettingsService),
                                                                            _artNetPubSub(LightState::dmxRead, LightState::read, LightState::update, this, artNetReceiver)
/*  _webSocketClient(LightState::read,
                   LightState::update,
                   this,
                   LIGHT_SETTINGS_SOCKET_PATH)*/
{
    // configure led to be output
    pinMode(LED_BUILTIN, OUTPUT);

#if FT_MQTT
    // configure MQTT callback
    _mqttClient->onConnect(std::bind(&LightStateService::registerConfig, this));

    // configure update handler for when the light settings change
    _lightMqttSettingsService->addUpdateHandler([&](const String &originId)
                                                { registerConfig(); },
                                                false);
#endif

    // configure update handler for when the light settings change
    _lightArtNetSettingsService->addUpdateHandler([&](const String &originId)
                                                { configureArtNet(); },
                                                false);

    // configure settings service update handler to update LED state
    addUpdateHandler([&](const String &originId)
                     { onConfigUpdated(); },
                     false);
}

void LightStateService::begin()
{
    _httpEndpoint.begin();
    _webSocketServer.begin();
    _artNetPubSub.begin();
    _state.ledOn = DEFAULT_LED_STATE;
    _state.brightness = DEFAULT_BRIGHTNESS;
    onConfigUpdated();
    configureArtNet();
}

void LightStateService::onConfigUpdated()
{
    analogWrite(LED_BUILTIN, _state.ledOn ? _state.brightness : 0);
}

#if FT_MQTT
void LightStateService::registerConfig()
{
    if (!_mqttClient->connected())
    {
        return;
    }
    String configTopic;
    String subTopic;
    String pubTopic;

    DynamicJsonDocument doc(256);
    _lightMqttSettingsService->read([&](LightMqttSettings &settings)
                                    {
    configTopic = settings.mqttPath + "/config";
    subTopic = settings.mqttPath + "/set";
    pubTopic = settings.mqttPath + "/state";
    doc["~"] = settings.mqttPath;
    doc["name"] = settings.name;
    doc["unique_id"] = settings.uniqueId; });
    doc["cmd_t"] = "~/set";
    doc["stat_t"] = "~/state";
    doc["schema"] = "json";
    doc["brightness"] = true;

    String payload;
    serializeJson(doc, payload);
    _mqttClient->publish(configTopic.c_str(), 0, false, payload.c_str());

    _mqttPubSub.configureTopics(pubTopic, subTopic);
}
#endif

void LightStateService::configureArtNet() {
    _artNetPubSub.address = _lightArtNetSettingsService->getAddress();
    _artNetPubSub.setUniverse(_lightArtNetSettingsService->getUniverse());
}