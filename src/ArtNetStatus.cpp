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

#include <ArtNetStatus.h>

ArtNetStatus::ArtNetStatus(PsychicHttpServer *server, ArtNetSettingsService *artNetSettingsService, SecurityManager *securityManager) : _server(server),
                                                                                                                                _securityManager(securityManager),
                                                                                                                                _artNetSettingsService(artNetSettingsService)
{
}

void ArtNetStatus::begin()
{
    _server->on(ARTNET_STATUS_SERVICE_PATH,
                HTTP_GET,
                _securityManager->wrapRequest(std::bind(&ArtNetStatus::artNetStatus, this, std::placeholders::_1),
                                              AuthenticationPredicates::IS_AUTHENTICATED));

    ESP_LOGV("ArtNetStatus", "Registered GET endpoint: %s", ARTNET_STATUS_SERVICE_PATH);
}

esp_err_t ArtNetStatus::artNetStatus(PsychicRequest *request)
{
    PsychicJsonResponse response = PsychicJsonResponse(request, false, MAX_ARTNET_STATUS_SIZE);
    JsonObject root = response.getRoot();

    root["enabled"] = _artNetSettingsService->isEnabled();
    root["connected"] = _artNetSettingsService->isConnected();
    root["client_id"] = _artNetSettingsService->getClientId();
    root["last_error"] = _artNetSettingsService->getLastError();

    return response.send();
}
