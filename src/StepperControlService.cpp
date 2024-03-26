#include <StepperControlService.h>

StepperControlService::StepperControlService(PsychicHttpServer *server,
                                     SecurityManager *securityManager,
                                     StepperArtNetSettingsService *stepperArtNetSettingsService,
                                     ArtnetWiFiReceiver *artNetReceiver,
                                     TMC5160Controller *stepper) :                  
                                                                            _httpEndpoint(StepperControl::read,
                                                                                            StepperControl::update,
                                                                                            this,
                                                                                            server,
                                                                                            STEPPER_CONTROL_ENDPOINT_PATH,
                                                                                            securityManager,
                                                                                            AuthenticationPredicates::IS_AUTHENTICATED),
                                                                            _webSocketServer(StepperControl::read,
                                                                                                StepperControl::update,
                                                                                                this,
                                                                                                server,
                                                                                                STEPPER_CONTROL_SOCKET_PATH,
                                                                                                securityManager,
                                                                                                AuthenticationPredicates::IS_AUTHENTICATED),
                                                                            _stepperArtNetSettingsService(stepperArtNetSettingsService),
                                                                            _artNetPubSub(StepperControl::dmxRead, StepperControl::read, StepperControl::update, this, artNetReceiver),
                                                                            _stepper(stepper)
{
    // configure led to be output
    pinMode(LED_BUILTIN, OUTPUT);

    // configure update handler for when the stepper settings change
    _stepperArtNetSettingsService->addUpdateHandler([&](const String &originId)
                                                { configureArtNet(); },
                                                false);

    // configure settings service update handler to update LED state
    addUpdateHandler([&](const String &originId)
                     { onConfigUpdated(); },
                     false);
}

void StepperControlService::begin()
{
    _httpEndpoint.begin();
    _webSocketServer.begin();
    _artNetPubSub.begin();
    _stepper->init();
    // _state.ledOn = DEFAULT_LED_STATE;
    // _state.brightness = DEFAULT_BRIGHTNESS;
    onConfigUpdated();
    configureArtNet();
    updateState();
}

void StepperControlService::onConfigUpdated()
{
    if (_state.isEnabled & !_stepper->enabled) _stepper->enable();
    else if(!_state.isEnabled & _stepper->enabled) _stepper->disable();
    _stepper->setAcceleration(_state.acceleration);
    _stepper->setSpeed(_state.direction ? _state.speed : -_state.speed);
    if (_state.move) _stepper->move(_state.direction ? _state.move : -_state.move);
    _stepper->driver.rms_current(_state.current);
}

void StepperControlService::updateState() {
    
    DynamicJsonDocument json(4096);
    JsonObject jsonObject = json.to<JsonObject>();
    _state.readState(_stepper, jsonObject);
    update(jsonObject, _state.update, "timer");
}

void StepperControlService::configureArtNet() {
    _artNetPubSub.address = _stepperArtNetSettingsService->getAddress();
    _artNetPubSub.setUniverse(_stepperArtNetSettingsService->getUniverse());
}