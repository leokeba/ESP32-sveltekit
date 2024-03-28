#include <StepperControlService.h>

StepperControlService::StepperControlService(PsychicHttpServer *server,
                                     SecurityManager *securityManager,
                                     StepperArtNetSettingsService *stepperArtNetSettingsService,
                                     StepperSettingsService *stepperSettingsService,
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
                                                                            _stepperSettingsService(stepperSettingsService),
                                                                            _artNetPubSub(StepperControl::dmxRead, StepperControl::read, StepperControl::update, this, artNetReceiver),
                                                                            _stepper(stepper)
{
    // configure update handler for when the stepper settings change
    _stepperArtNetSettingsService->addUpdateHandler([&](const String &originId)
                                                { configureArtNet(); },
                                                false);
    
    // _stepperSettingsService->addUpdateHandler([&](const String &originId)
    //                                             { onConfigUpdated(originId); },
    //                                             false);

    addUpdateHandler([&](const String &originId)
                     { onConfigUpdated(originId); },
                     false);
}

void StepperControlService::begin()
{
    _httpEndpoint.begin();
    _webSocketServer.begin();
    _artNetPubSub.begin();
    updateState();
    onConfigUpdated("begin");
    configureArtNet();
}

void StepperControlService::loop() {
    unsigned long now = millis();
    if (now - lastUpdate > 500) {
        updateState();
        lastUpdate = now;
    }
}

void StepperControlService::onConfigUpdated(const String &originId)
{
    // _state.settings = _stepperSettingsService->getState();
    if (originId != "driver") {
        Serial.println(originId);
        if (_state.isEnabled & !_stepper->enabled) _stepper->enable();
        else if(!_state.isEnabled & _stepper->enabled) _stepper->disable();
        _stepper->setAcceleration(_state.acceleration);
        _stepper->setSpeed(_state.direction ? _state.speed : -_state.speed);
        if (abs(_state.newMove)>0.01) _stepper->move(_state.newMove*360.);
    }
}

void StepperControlService::updateState() {
    DynamicJsonDocument json(1024);
    JsonObject jsonObject = json.to<JsonObject>();
    _state.readState(_stepper, _stepperSettingsService, jsonObject);
    update(jsonObject, _state.update, "driver");
}

void StepperControlService::configureArtNet() {
    _artNetPubSub.address = _stepperArtNetSettingsService->getAddress();
    _artNetPubSub.setUniverse(_stepperArtNetSettingsService->getUniverse());
}