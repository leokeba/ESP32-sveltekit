#include <ArtNetSettingsService.h>

ArtNetSettingsService::ArtNetSettingsService(PsychicHttpServer *server, 
                                            FS *fs, SecurityManager *securityManager, 
                                            ArtnetWiFiReceiver *artNetReceiver) : 
                                                                                _server(server),
                                                                                _securityManager(securityManager),
                                                                                _httpEndpoint(ArtNetSettings::read, ArtNetSettings::update, this, server, ARTNET_SETTINGS_SERVICE_PATH, securityManager),
                                                                                _fsPersistence(ArtNetSettings::read, ArtNetSettings::update, this, fs, ARTNET_SETTINGS_FILE),
                                                                                _artNetReceiver(artNetReceiver)
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

    _httpEndpoint.begin();
    _fsPersistence.readFromFS();
}

void ArtNetSettingsService::loop()
{
    if (_state.enabled && WiFi.isConnected() && _hasBegun) _artNetReceiver->parse();
}

bool ArtNetSettingsService::isEnabled()
{
    return _state.enabled;
}

int ArtNetSettingsService::getPort()
{
    return _state.port;
}

void ArtNetSettingsService::onConfigUpdated()
{
    // only connect if WiFi is connected and ARTNET is enabled
    if (_state.enabled && WiFi.isConnected())
    {
#ifdef SERIAL_INFO
        Serial.println("Connecting to ARTNET...");
#endif
        _artNetReceiver->begin(_state.port);
        _hasBegun = true;
    }
}

void ArtNetSettingsService::onStationModeGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    if (_state.enabled)
    {
        ESP_LOGI("ARTNET", "WiFi connection established, starting ARTNET client.");
        onConfigUpdated();
    }
}

void ArtNetSettingsService::onStationModeDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    if (_state.enabled)
    {
        ESP_LOGI("ARTNET", "WiFi connection dropped, stopping ARTNET client.");
        onConfigUpdated();
    }
}