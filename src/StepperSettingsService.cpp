#include <StepperSettingsService.h>

StepperSettingsService::StepperSettingsService(PsychicHttpServer *server,
                                                FS *fs,
                                                SecurityManager *securityManager,
                                                TMC5160Controller *stepper) :                  
                                                                            _httpEndpoint(StepperSettings::read,
                                                                                            StepperSettings::update,
                                                                                            this,
                                                                                            server,
                                                                                            STEPPER_SETTINGS_ENDPOINT_PATH,
                                                                                            securityManager,
                                                                                            AuthenticationPredicates::IS_AUTHENTICATED),
                                                                            _fsPersistence(StepperSettings::read, StepperSettings::update, this, fs, STEPPER_SETTINGS_FILE),
                                                                            _stepper(stepper)
{
    // configure led to be output
    pinMode(LED_BUILTIN, OUTPUT);

    // configure settings service update handler to update LED state
    addUpdateHandler([&](const String &originId)
                     { onConfigUpdated(); },
                     false);
}

void StepperSettingsService::begin()
{
    _httpEndpoint.begin();
    _fsPersistence.readFromFS();
    // _stepper->init();
    onConfigUpdated();
    if (_state.enableOnStart) _stepper->enable();
    else _stepper->disable();
    // updateState();
}

int32_t StepperSettingsService::getMaxSpeed() {
    return _state.maxSpeed;
}

int32_t StepperSettingsService::getMaxAccel() {
    return _state.maxAcceleration;
}

StepperSettings StepperSettingsService::getState() {
    return _state;
}

void StepperSettingsService::onConfigUpdated()
{
    _stepper->maxSpeed = _state.maxSpeed;
    _stepper->maxAccel = _state.maxAcceleration;
    _stepper->driver.rms_current(_state.current);
    _stepper->driver.shaft(_state.invertDirection);
}

// void StepperSettingsService::updateState() {
    
//     DynamicJsonDocument json(1024);
//     JsonObject jsonObject = json.to<JsonObject>();
//     _state.readState(_stepper, jsonObject);
//     update(jsonObject, _state.update, "timer");
// }