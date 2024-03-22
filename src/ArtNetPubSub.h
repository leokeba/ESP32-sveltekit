#ifndef ArtNetPubSub_h
#define ArtNetPubSub_h

/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2023 - 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <StatefulService.h>

#include <ArtnetWiFi.h>

class ArtNetPubSub
{
public:
    ArtNetPubSub(ArtnetWiFiReceiver *artNetReceiver) : _artNetReceiver(artNetReceiver)

    {
        // _statefulService->addUpdateHandler([&](const String &originId)
        //                                    { publish(); },
        //                                    false);

    }

    void begin() {

        _artNetReceiver->begin();

        // if Artnet packet comes to this universe, this function (lambda) is called
        _artNetReceiver->subscribeArtDmxUniverse(1, [&](const uint8_t *data, uint16_t size, const ArtDmxMetadata &metadata, const ArtNetRemoteInfo &remote) {
            recvCallback(data, size, metadata, remote);
        });
    }

    void recvCallback(const uint8_t *data, uint16_t size, const ArtDmxMetadata &metadata, const ArtNetRemoteInfo &remote) {
        Serial.print("lambda : artnet data from ");
        Serial.print(remote.ip);
        Serial.print(":");
        Serial.print(remote.port);
        Serial.print(", size = ");
        Serial.print(size);
        Serial.print(") :");
        for (size_t i = 0; i < size; ++i) {
            Serial.print(data[i]);
            Serial.print(",");
        }
        Serial.println();
    }

    void loop() {
        _artNetReceiver->parse();
    }

protected:
    ArtnetWiFiReceiver *_artNetReceiver;
    int _bufferSize;
};

#endif // end ArtNetPubSub
