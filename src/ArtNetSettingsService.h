#ifndef ArtNetSettingsService_h
#define ArtNetSettingsService_h

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

#include <StatefulService.h>
#include <HttpEndpoint.h>
#include <FSPersistence.h>
#include <SettingValue.h>
#include <WiFi.h>

#ifndef FACTORY_ARTNET_ENABLED
#define FACTORY_ARTNET_ENABLED false
#endif

#define ARTNET_SETTINGS_FILE "/config/mqttSettings.json"
#define ARTNET_SETTINGS_SERVICE_PATH "/rest/mqttSettings"

class ArtNetSettings
{
public:
    // host and port - if enabled
    bool enabled;
    String uri;

    // username and password
    String username;
    String password;

    // client id settings
    String clientId;

    // connection settings
    uint16_t keepAlive;
    bool cleanSession;

    static void read(ArtNetSettings &settings, JsonObject &root)
    {
        root["enabled"] = settings.enabled;
        root["uri"] = settings.uri;
        root["username"] = settings.username;
        root["password"] = settings.password;
        root["client_id"] = settings.clientId;
        root["keep_alive"] = settings.keepAlive;
        root["clean_session"] = settings.cleanSession;
    }

    static StateUpdateResult update(JsonObject &root, ArtNetSettings &settings)
    {
        settings.enabled = root["enabled"] | FACTORY_ARTNET_ENABLED;
        return StateUpdateResult::CHANGED;
    }
};

class ArtNetSettingsService : public StatefulService<ArtNetSettings>
{
public:
    ArtNetSettingsService(PsychicHttpServer *server, FS *fs, SecurityManager *securityManager);
    ~ArtNetSettingsService();

    void begin();
    void loop();
    bool isEnabled();
    bool isConnected();
    const char *getClientId();
    String getLastError();

protected:
    void onConfigUpdated();

private:
    PsychicHttpServer *_server;
    SecurityManager *_securityManager;
    HttpEndpoint<ArtNetSettings> _httpEndpoint;
    FSPersistence<ArtNetSettings> _fsPersistence;

    // Pointers to hold retained copies of the mqtt client connection strings.
    // This is required as AsyncArtNetClient holds references to the supplied connection strings.
    char *_retainedHost;
    char *_retainedClientId;
    char *_retainedUsername;
    char *_retainedPassword;

    // variable to help manage connection
    bool _reconfigureArtNet;
    String _lastError;

    void onStationModeGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
    void onStationModeDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);

    void onArtNetConnect(bool sessionPresent);
    void onArtNetDisconnect(bool sessionPresent);
    // void onArtNetError(esp_mqtt_error_codes_t error);
    void configureArtNet();
};

#endif // end ArtNetSettingsService_h
