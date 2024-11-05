#include <ClosedLoopControllerService.h>

ClosedLoopControllerStateService::ClosedLoopControllerStateService(EventSocket *socket,
                                                                   ClosedLoopController* controller,
                                                                   int index = 0) :
                                                                            _eventEndpoint(ClosedLoopControllerState::read,
                                                                                            ClosedLoopControllerState::update,
                                                                                            this,
                                                                                            socket,
                                                                                            strdup((String(CL_CONTROLLER_STATE_EVENT)+String(index)).c_str())),
                                                                            _controller(controller)
{
    addUpdateHandler([&](const String &originId)
                     { onConfigUpdated(originId); },
                     false);
}

void ClosedLoopControllerStateService::begin()
{
    _eventEndpoint.begin();
    if (_controller->enabled) _controller->setAngle(_controller->getAngle());
    updateState();
    onConfigUpdated("begin");
}

void ClosedLoopControllerStateService::loop() {
    bool changed = _controller->encoder.update();
    _controller->run();
    // if (changed) updateState();
    updateState();
}

void ClosedLoopControllerStateService::onConfigUpdated(const String &originId)
{
    if (originId != "stateUpdate") {
        _controller->targetAngle = _state.targetAngle;
    }
}

void ClosedLoopControllerStateService::updateState() {
    JsonDocument json;
    JsonObject jsonObject = json.to<JsonObject>();
    _state.readState(_controller, jsonObject);
    update(jsonObject, _state.update, "stateUpdate");
}

ClosedLoopControllerSettingsService::ClosedLoopControllerSettingsService(EventSocket *socket,
                                                                         FS *fs,
                                                                         ClosedLoopController* controller,
                                                                         int index = 0) :                  
                                                                                _eventEndpoint(ClosedLoopControllerSettings::read,
                                                                                                ClosedLoopControllerSettings::update,
                                                                                                this,
                                                                                                socket,
                                                                                                strdup((String(CL_CONTROLLER_SETTINGS_EVENT)+String(index)).c_str())),
                                                                                _fsPersistence(ClosedLoopControllerSettings::read, 
                                                                                                ClosedLoopControllerSettings::update, 
                                                                                                this, 
                                                                                                fs, 
                                                                                                "/config/controllerSettings.json"),
                                                                                _controller(controller)
{
    // configure settings service update handler to update LED state
    addUpdateHandler([&](const String &originId)
                     { onConfigUpdated(); },
                     false);
}

void ClosedLoopControllerSettingsService::begin()
{
    _eventEndpoint.begin();
    _fsPersistence.readFromFS();
    onConfigUpdated();
}


void ClosedLoopControllerSettingsService::onConfigUpdated()
{
    _controller->enabled = _state.enabled;
    _controller->hasLimits = _state.hasLimits;
    _controller->tolerance = _state.tolerance;
    _controller->limitA = _state.limitA;
    _controller->limitB = _state.limitB;
}

MultiClosedLoopControllerService::MultiClosedLoopControllerService(EventSocket *socket,
                                                                   FS *fs,
                                                                   std::vector<ClosedLoopController*>& controllers) :
                                                                                _eventEndpoint(ClosedLoopControllerMap::read,
                                                                                                ClosedLoopControllerMap::update,
                                                                                                this,
                                                                                                socket,
                                                                                                CL_CONTROLLER_STATE_EVENT),
                                                                                _controllers(controllers)

{
    for (int i = 0; i < _controllers.size(); i++) {
        auto stateService = ClosedLoopControllerStateService(socket, _controllers[i], i);
        _stateServices.push_back(stateService);
        auto settingsService = ClosedLoopControllerSettingsService(socket, fs, _controllers[i], i);
        _settingsServices.push_back(settingsService);
    }
}

void MultiClosedLoopControllerService::begin()
{
    for (auto state : _stateServices) state.begin();
    for (auto settings : _settingsServices) settings.begin();
}

void MultiClosedLoopControllerService::loop()
{
    // for (auto state : _stateServices) state.loop();
}