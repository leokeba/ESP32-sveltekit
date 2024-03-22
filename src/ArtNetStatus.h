#ifndef ArtNetStatus_h
#define ArtNetStatus_h

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

#include <WiFi.h>

#include <ArtNetSettingsService.h>
#include <ArduinoJson.h>
#include <PsychicHttp.h>
#include <SecurityManager.h>

#define MAX_ARTNET_STATUS_SIZE 1024
#define ARTNET_STATUS_SERVICE_PATH "/rest/artNetStatus"

class ArtNetStatus
{
public:
    ArtNetStatus(PsychicHttpServer *server, ArtNetSettingsService *artNetSettingsService, SecurityManager *securityManager);

    void begin();

private:
    PsychicHttpServer *_server;
    SecurityManager *_securityManager;
    ArtNetSettingsService *_artNetSettingsService;

    esp_err_t artNetStatus(PsychicRequest *request);
};

#endif // end ArtNetStatus_h
