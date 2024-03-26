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
#include <StepperControlService.h>

#define SERIAL_BAUD_RATE 115200

PsychicHttpServer server;

ESP32SvelteKit esp32sveltekit(&server, 120);

FastAccelStepperEngine engine = FastAccelStepperEngine();

TMC5160Stepper driver1(5, R_SENSE, MOSI, MISO, SCK);
// TMC5160Stepper driver2(16, R_SENSE, MOSI, MISO, SCK);

TMC5160Controller stepper1 = {driver1, engine, 21, 17, EN_PIN};
// TMC5160Controller stepper2 = {driver2, STEP_PIN, DIR_PIN, EN_PIN};


#if FT_ENABLED(FT_MQTT)
LightMqttSettingsService lightMqttSettingsService =
    LightMqttSettingsService(&server, esp32sveltekit.getFS(), esp32sveltekit.getSecurityManager());
#endif

// LightArtNetSettingsService lightArtNetSettingsService =
//     LightArtNetSettingsService(&server, esp32sveltekit.getFS(), esp32sveltekit.getSecurityManager());


StepperArtNetSettingsService stepperArtNetSettingsService =
    StepperArtNetSettingsService(&server, esp32sveltekit.getFS(), esp32sveltekit.getSecurityManager());

ArtnetWiFiReceiver artNetReceiver;

StepperControlService stepperControlService = StepperControlService(&server,
                                                        esp32sveltekit.getSecurityManager(),
                                                        &stepperArtNetSettingsService,
                                                        &artNetReceiver,
                                                        &stepper1
);

// LightStateService lightStateService = LightStateService(&server,
//                                                         esp32sveltekit.getSecurityManager(),
// #if FT_ENABLED(FT_MQTT)
//                                                         esp32sveltekit.getMqttClient(),
//                                                         &lightMqttSettingsService,
// #endif
//                                                         &lightArtNetSettingsService,
//                                                         &artNetReceiver
// );

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
#if FT_ENABLED(FT_MQTT)
    lightMqttSettingsService.begin();
#endif
    // lightArtNetSettingsService.begin();
    // start the light service
    // lightStateService.begin();

    // artNetDataService.begin();

    stepperArtNetSettingsService.begin();

    stepperControlService.begin();
}

void loop()
{
    // Delete Arduino loop task, as it is not needed in this example
    // vTaskDelete(NULL);
    // artNetReceiver.parse();
    artNetSettingsService.loop();
}