#ifndef ArtNetDataService_h
#define ArtNetDataService_h

#include <StatefulService.h>
#include <HttpEndpoint.h>
#include <ArtNetPubSub.h>

#define ARTNET_DATA_ENDPOINT_PATH "/rest/artNetData"

class ArtNetFrame
{
public:

    static const uint16_t dmxChannels = 16;

    uint8_t data[dmxChannels] = {};
    uint16_t size = dmxChannels;
    
    static void read(ArtNetFrame &settings, JsonObject &root)
    {
        std::string arrayStr = std::string((char *)settings.data, settings.size);
        root["arrayStr"] = arrayStr;
        root["size"] = min(settings.size, settings.dmxChannels);
    }

    static StateUpdateResult update(JsonObject &root, ArtNetFrame &settings)
    {
        std::string arrayStr = root["arrayStr"];
        uint16_t size = root["size"];
        size = min(size, settings.dmxChannels);
        settings.size = size;
        for (int i = 0; i < size; i++) {
            settings.data[i] = (uint8_t) arrayStr[i];
        }
        return StateUpdateResult::CHANGED;
    }

    static void readArtNet(DmxFrame &data, JsonObject &root) {
        std::string arrayStr = std::string((char *)data.data, data.size);
        root["arrayStr"] = arrayStr;
        root["size"] = data.size;
        // serializeJson(root, Serial);
    }
};

class ArtNetDataService : public StatefulService<ArtNetFrame>
{
public:
    ArtNetDataService(PsychicHttpServer *server, SecurityManager *securityManager, ArtnetWiFiReceiver *artNetReceiver);
    ~ArtNetDataService();

    void begin();

protected:
    void onDataUpdated();

private:
    PsychicHttpServer *_server;
    SecurityManager *_securityManager;
    ArtnetWiFiReceiver *_artNetReceiver;
    HttpEndpoint<ArtNetFrame> _httpEndpoint;
    ArtNetPubSub<ArtNetFrame> _artNetPubSub;
};

#endif // end ArtNetDataService
