#ifndef ArtNetDataService_h
#define ArtNetDataService_h

#include <StatefulService.h>
#include <HttpEndpoint.h>
#include <ArtNetPubSub.h>

#define ARTNET_DATA_ENDPOINT_PATH "/rest/artNetData"

class ArtNetFrame
{
public:
    uint8_t data[512] = {};
    uint16_t size = 512;
    
    static void read(ArtNetFrame &settings, JsonObject &root)
    {
        std::string arrayStr = std::string((char *)settings.data, settings.size);
        root["arrayStr"] = arrayStr;
        root["size"] = settings.size;
    }

    static void readArtNet(DmxFrame &data, JsonObject &root) {
        std::string arrayStr = std::string((char *)data.data, data.size);
        root["arrayStr"] = arrayStr;
        root["size"] = data.size;
        // serializeJson(root, Serial);
    }

    static StateUpdateResult update(JsonObject &root, ArtNetFrame &frame)
    {
        std::string arrayStr = root["arrayStr"];
        uint16_t size = root["size"];
        frame.size = size;
        for (int i = 0; i < size; i++) {
            frame.data[i] = (uint8_t) arrayStr[i];
        }
        return StateUpdateResult::CHANGED;
    }
};

class ArtNetDataService : public StatefulService<ArtNetFrame>
{
public:
    ArtNetDataService(PsychicHttpServer *server, SecurityManager *securityManager, ArtnetWiFiReceiver *artNetReceiver);
    ~ArtNetDataService();

    void begin();
    void loop();

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
