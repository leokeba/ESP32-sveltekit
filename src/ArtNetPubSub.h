#ifndef ArtNetPubSub_h
#define ArtNetPubSub_h

#include <StatefulService.h>
#include <HttpEndpoint.h>

#include <ArtnetWiFi.h>
#include <ArtNetDataService.h>

#define ARTNET_DATA_ENDPOINT_PATH "/rest/artNetData"

using ArtNetData = std::vector<uint8_t>;

struct DmxFrame
{
    const uint8_t *data;
    uint16_t size;
};

template <class T>
class ArtNetPubSub
{
public:
    const uint8_t universe = 1;
    const uint16_t length = 8;
    uint16_t address = 0;
    ArtNetPubSub(
                JsonStateReader<DmxFrame> artNetReader,
                JsonStateReader<T> stateReader,
                JsonStateUpdater<T> stateUpdater,
                StatefulService<T> *statefulService,
                ArtnetWiFiReceiver *artNetReceiver) :   _artNetReader(artNetReader),
                                                        _stateReader(stateReader),
                                                        _stateUpdater(stateUpdater),
                                                        _statefulService(statefulService),
                                                        _artNetReceiver(artNetReceiver)

    {
        // _statefulService->addUpdateHandler([&](const String &originId)
        //                                    { publish(); },
        //                                    false);
    }

    void begin() {

        _artNetReceiver->subscribeArtDmxUniverse(universe, [&](const uint8_t *data, uint16_t size, const ArtDmxMetadata &metadata, const ArtNetRemoteInfo &remote) {
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
        const uint16_t end = min(size, uint16_t(address+length));
        uint8_t newData[length] = {};
        // ArtNetData artNetData;
        for (int i = address; i < end; i++) {
            newData[i-address] = data[i];
            // artNetData.push_back(data[i]);
        }
        DmxFrame dmxFrame = {data, length};
        DynamicJsonDocument json(4096);
        JsonObject jsonObject = json.to<JsonObject>();
        _artNetReader(dmxFrame, jsonObject);
        // serializeJson(json, Serial);
        _statefulService->update(jsonObject, _stateUpdater, "artnet");
    }

    DynamicJsonDocument stringFromFrame(const uint8_t *data, const uint16_t size) {
        std::string arrayStr = std::string((char *)data, size);
        DynamicJsonDocument json(size*6+64);
        json["data"] = arrayStr;
        json["size"] = size;
        return json;
    }

    void loop() {
        _artNetReceiver->parse();
    }

protected:
    StatefulService<T> *_statefulService;
    JsonStateUpdater<T> _stateUpdater;
    JsonStateReader<T> _stateReader;
    JsonStateReader<DmxFrame> _artNetReader;
    ArtnetWiFiReceiver *_artNetReceiver;
};

#endif // end ArtNetPubSub
