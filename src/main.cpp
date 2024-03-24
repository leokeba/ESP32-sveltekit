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

#include <ESP32SvelteKit.h>
#include <LightMqttSettingsService.h>
#include <LightStateService.h>
#include <PsychicHttpServer.h>
#include <ArtNetStatus.h>
// #include <ArtNetDataService.h>

#define SERIAL_BAUD_RATE 115200

PsychicHttpServer server;

ESP32SvelteKit esp32sveltekit(&server, 120);

LightMqttSettingsService lightMqttSettingsService =
    LightMqttSettingsService(&server, esp32sveltekit.getFS(), esp32sveltekit.getSecurityManager());

LightArtNetSettingsService lightArtNetSettingsService =
    LightArtNetSettingsService(&server, esp32sveltekit.getFS(), esp32sveltekit.getSecurityManager());

ArtnetWiFiReceiver artNetReceiver;

LightStateService lightStateService = LightStateService(&server,
                                                        esp32sveltekit.getSecurityManager(),
                                                        esp32sveltekit.getMqttClient(),
                                                        &lightMqttSettingsService,
                                                        &lightArtNetSettingsService,
                                                        &artNetReceiver);

ArtNetSettingsService artNetSettingsService = ArtNetSettingsService(&server,
                                                                    esp32sveltekit.getFS(),
                                                                    esp32sveltekit.getSecurityManager(),
                                                                    &artNetReceiver);

ArtNetStatus artNetStatus = ArtNetStatus(&server,
                                        &artNetSettingsService,
                                        esp32sveltekit.getSecurityManager());

// ArtNetDataService artNetDataService = ArtNetDataService(&server,
//                                                         esp32sveltekit.getSecurityManager(),
//                                                         &artNetReceiver);

void setup()
{
    // start serial and filesystem
    Serial.begin(SERIAL_BAUD_RATE);

    // start ESP32-SvelteKit
    esp32sveltekit.begin();

    artNetSettingsService.begin();
    artNetStatus.begin();
    
    // artNetReceiver.begin();

    // load the initial light settings
    lightMqttSettingsService.begin();
    lightArtNetSettingsService.begin();
    // start the light service
    lightStateService.begin();

    // artNetDataService.begin();
}

void loop()
{
    // Delete Arduino loop task, as it is not needed in this example
    // vTaskDelete(NULL);
    // artNetReceiver.parse();
    artNetSettingsService.loop();
}