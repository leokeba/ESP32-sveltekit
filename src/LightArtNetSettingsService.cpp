#include <LightArtNetSettingsService.h>

LightArtNetSettingsService::LightArtNetSettingsService(PsychicHttpServer *server, FS *fs, SecurityManager *securityManager) : _httpEndpoint(LightArtNetSettings::read,
                                                                                                                                        LightArtNetSettings::update,
                                                                                                                                        this,
                                                                                                                                        server,
                                                                                                                                        LIGHT_ARTNET_SETTINGS_PATH,
                                                                                                                                        securityManager,
                                                                                                                                        AuthenticationPredicates::IS_AUTHENTICATED),
                                                                                                                          _fsPersistence(LightArtNetSettings::read, LightArtNetSettings::update, this, fs, LIGHT_ARTNET_SETTINGS_FILE)
{
}

void LightArtNetSettingsService::begin()
{
    _httpEndpoint.begin();
    _fsPersistence.readFromFS();
}

uint16_t LightArtNetSettingsService::getAddress() {
    return _state.channel;
}

uint16_t LightArtNetSettingsService::getUniverse() {
    return _state.universe;
}