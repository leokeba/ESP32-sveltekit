#ifndef ArtNetDataService_h
#define ArtNetDataService_h

#include <StatefulService.h>
#include <HttpEndpoint.h>
#include <ArtNetPubSub.h>

#define ARTNET_DATA_ENDPOINT_PATH "/rest/artNetData"

class ArtNetFrame
{
public:
    uint8_t data[512];
    static void read(ArtNetFrame &settings, JsonObject &root)
    {
        
        JsonArray array = root["data"].to<JsonArray>();
        for (int i = 0; i < 512; i++) {
            array.add(settings.data[i]);
        }
    }

    static StateUpdateResult update(JsonObject &root, ArtNetFrame &frame)
    {
        JsonArray newData = root["data"];
        for (int i = 0; i < 512; i++) {
            frame.data[i] = newData[i];
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
