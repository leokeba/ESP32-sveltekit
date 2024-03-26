#include <StepperArtNetSettingsService.h>

StepperArtNetSettingsService::StepperArtNetSettingsService(PsychicHttpServer *server, FS *fs, SecurityManager *securityManager) : _httpEndpoint(StepperArtNetSettings::read,
                                                                                                                                        StepperArtNetSettings::update,
                                                                                                                                        this,
                                                                                                                                        server,
                                                                                                                                        STEPPER_ARTNET_SETTINGS_PATH,
                                                                                                                                        securityManager,
                                                                                                                                        AuthenticationPredicates::IS_AUTHENTICATED),
                                                                                                                          _fsPersistence(StepperArtNetSettings::read, StepperArtNetSettings::update, this, fs, STEPPER_ARTNET_SETTINGS_FILE)
{
}

void StepperArtNetSettingsService::begin()
{
    _httpEndpoint.begin();
    _fsPersistence.readFromFS();
}

uint16_t StepperArtNetSettingsService::getAddress() {
    return _state.channel;
}

uint16_t StepperArtNetSettingsService::getUniverse() {
    return _state.universe;
}