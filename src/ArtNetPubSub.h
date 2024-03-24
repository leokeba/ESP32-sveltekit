#ifndef ArtNetPubSub_h
#define ArtNetPubSub_h

#include <StatefulService.h>
#include <HttpEndpoint.h>

#include <ArtnetWiFi.h>

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
        const uint16_t end = min(size, uint16_t(address+length));
        uint8_t newData[length] = {};
        for (int i = address; i < end; i++) {
            newData[i-address] = data[i];
        }
        DmxFrame dmxFrame = {data, length};
        DynamicJsonDocument json(4096);
        JsonObject jsonObject = json.to<JsonObject>();
        _artNetReader(dmxFrame, jsonObject);
        // serializeJson(json, Serial);
        _statefulService->update(jsonObject, _stateUpdater, "artnet");
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
