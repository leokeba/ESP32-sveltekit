#ifndef ArtNetPubSub_h
#define ArtNetPubSub_h

#include <StatefulService.h>
#include <HttpEndpoint.h>

#include <ArtnetWiFi.h>
#include <ArtNetDataService.h>

#define ARTNET_DATA_ENDPOINT_PATH "/rest/artNetData"

template <class T>
class ArtNetPubSub
{
public:
    ArtNetPubSub(JsonStateReader<T> stateReader,
               JsonStateUpdater<T> stateUpdater,
               StatefulService<T> *statefulService,
               ArtnetWiFiReceiver *artNetReceiver) :    _stateReader(stateReader),
                                                        _stateUpdater(stateUpdater),
                                                        _statefulService(statefulService),
                                                        _artNetReceiver(artNetReceiver)

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
        // Serial.print("lambda : artnet data from ");
        // Serial.print(remote.ip);
        // Serial.print(":");
        // Serial.print(remote.port);
        // Serial.print(", size = ");
        // Serial.print(size);
        // Serial.print(") :");
        // for (size_t i = 0; i < size; ++i) {
        //     Serial.print(data[i]);
        //     Serial.print(",");
        // }
        // Serial.println();
        std::string arrayStr = std::string((char *)data, size);
        DynamicJsonDocument json(size*6+64);
        json["data"] = arrayStr;
        json["size"] = size;
        if (json.is<JsonObject>())
        {
            JsonObject jsonObject = json.as<JsonObject>();
            _statefulService->update(jsonObject, _stateUpdater, "artnet");
        }
    }

    void loop() {
        _artNetReceiver->parse();
    }

protected:
    StatefulService<T> *_statefulService;
    JsonStateUpdater<T> _stateUpdater;
    JsonStateReader<T> _stateReader;
    ArtnetWiFiReceiver *_artNetReceiver;
};

#endif // end ArtNetPubSub
