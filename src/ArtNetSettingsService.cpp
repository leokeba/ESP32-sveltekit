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

#include <ArtNetSettingsService.h>

extern const uint8_t rootca_crt_bundle_start[] asm("_binary_src_certs_x509_crt_bundle_bin_start");

/**
 * Retains a copy of the cstr provided in the pointer provided using dynamic allocation.
 *
 * Frees the pointer before allocation and leaves it as nullptr if cstr == nullptr.
 */
static char *retainCstr(const char *cstr, char **ptr)
{
    // free up previously retained value if exists
    free(*ptr);
    *ptr = nullptr;

    // dynamically allocate and copy cstr (if non null)
    if (cstr != nullptr)
    {
        *ptr = (char *)malloc(strlen(cstr) + 1);
        strcpy(*ptr, cstr);
    }

    // return reference to pointer for convenience
    return *ptr;
}

ArtNetSettingsService::ArtNetSettingsService(PsychicHttpServer *server, FS *fs, SecurityManager *securityManager) : _server(server),
                                                                                                                _securityManager(securityManager),
                                                                                                                _httpEndpoint(ArtNetSettings::read, ArtNetSettings::update, this, server, MQTT_SETTINGS_SERVICE_PATH, securityManager),
                                                                                                                _fsPersistence(ArtNetSettings::read, ArtNetSettings::update, this, fs, MQTT_SETTINGS_FILE),
                                                                                                                _retainedHost(nullptr),
                                                                                                                _retainedClientId(nullptr),
                                                                                                                _retainedUsername(nullptr),
                                                                                                                _retainedPassword(nullptr),
                                                                                                                _reconfigureArtNet(false),
                                                                                                                    // _artNetClient(),
                                                                                                                _lastError("None")
{
    addUpdateHandler([&](const String &originId)
                     { onConfigUpdated(); },
                     false);

}

ArtNetSettingsService::~ArtNetSettingsService()
{
}

void ArtNetSettingsService::begin()
{
    WiFi.onEvent(
        std::bind(&ArtNetSettingsService::onStationModeDisconnected, this, std::placeholders::_1, std::placeholders::_2),
        WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.onEvent(std::bind(&ArtNetSettingsService::onStationModeGotIP, this, std::placeholders::_1, std::placeholders::_2),
                 WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
        // _artNetClient.onConnect(std::bind(&ArtNetSettingsService::onArtNetConnect, this, std::placeholders::_1));
        // _artNetClient.onDisconnect(std::bind(&ArtNetSettingsService::onArtNetDisconnect, this, std::placeholders::_1));
        // _artNetClient.onError(std::bind(&ArtNetSettingsService::onArtNetError, this, std::placeholders::_1));

    _httpEndpoint.begin();
    _fsPersistence.readFromFS();
}

void ArtNetSettingsService::loop()
{
    if (_reconfigureArtNet)
    {
        // reconfigure MQTT client
        configureArtNet();

        // clear the reconnection flags
        _reconfigureArtNet = false;
    }
}

bool ArtNetSettingsService::isEnabled()
{
    return _state.enabled;
}

bool ArtNetSettingsService::isConnected()
{
        // return _artNetClient.connected();
}

const char *ArtNetSettingsService::getClientId()
{
    return _state.clientId.c_str();
}


String ArtNetSettingsService::getLastError()
{
    return _lastError;
}

void ArtNetSettingsService::onArtNetConnect(bool sessionPresent)
{
        // ESP_LOGI("MQTT", "Connected to MQTT: %s", _artNetClient.getArtNetConfig()->uri);
#ifdef SERIAL_INFO
        // Serial.printf("Connected to MQTT: %s\n", _artNetClient.getArtNetConfig()->uri);
#endif
    _lastError = "None";
}

void ArtNetSettingsService::onArtNetDisconnect(bool sessionPresent)
{
    ESP_LOGI("MQTT", "Disconnected from MQTT.");
#ifdef SERIAL_INFO
    Serial.println("Disconnected from MQTT.");
#endif
}

// void ArtNetSettingsService::onArtNetError(esp_artNet_error_codes_t error)
// {
//     if (error.error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
//     {
//         _lastError = strerror(error.esp_transport_sock_errno);
//         ESP_LOGE("MQTT", "MQTT TCP error: %s", _lastError.c_str());
//     }
// }

void ArtNetSettingsService::onConfigUpdated()
{
    _reconfigureArtNet = true;
}

void ArtNetSettingsService::onStationModeGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    if (_state.enabled)
    {
        ESP_LOGI("MQTT", "WiFi connection established, starting MQTT client.");
        onConfigUpdated();
    }
}

void ArtNetSettingsService::onStationModeDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    if (_state.enabled)
    {
        ESP_LOGI("MQTT", "WiFi connection dropped, stopping MQTT client.");
        onConfigUpdated();
    }
}

void ArtNetSettingsService::configureArtNet()
{
    // disconnect if currently connected
        // _artNetClient.disconnect();

    // only connect if WiFi is connected and MQTT is enabled
    if (_state.enabled && WiFi.isConnected())
    {
#ifdef SERIAL_INFO
        Serial.println("Connecting to MQTT...");
#endif
            // _artNetClient.setServer(retainCstr(_state.uri.c_str(), &_retainedHost));
        if (_state.username.length() > 0)
        {
                // _artNetClient.setCredentials(
                // retainCstr(_state.username.c_str(), &_retainedUsername),
                // retainCstr(_state.password.length() > 0 ? _state.password.c_str() : nullptr, &_retainedPassword));
        }
        else
        {
                // _artNetClient.setCredentials(retainCstr(nullptr, &_retainedUsername), retainCstr(nullptr, &_retainedPassword));
        }
            // _artNetClient.setClientId(retainCstr(_state.clientId.c_str(), &_retainedClientId));
            // _artNetClient.setKeepAlive(_state.keepAlive);
            // _artNetClient.setCleanSession(_state.cleanSession);
            // _artNetClient.connect();
    }
}
