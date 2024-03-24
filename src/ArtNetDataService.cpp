#include <ArtNetDataService.h>

ArtNetDataService::ArtNetDataService(PsychicHttpServer *server, 
                                    SecurityManager *securityManager, 
                                    ArtnetWiFiReceiver *artNetReceiver) :   _server(server),
                                                                            _securityManager(securityManager),
                                                                            _artNetReceiver(artNetReceiver),
                                                                            _httpEndpoint(ArtNetFrame::read, ArtNetFrame::update, this, server, ARTNET_DATA_ENDPOINT_PATH, securityManager),
                                                                            _artNetPubSub(ArtNetFrame::readArtNet, ArtNetFrame::read, ArtNetFrame::update, this, _artNetReceiver)
{
    addUpdateHandler([&](const String &originId)
                        { onDataUpdated(); },
                        false);
}

ArtNetDataService::~ArtNetDataService()
{
}

void ArtNetDataService::begin()
{
    _httpEndpoint.begin();
    _artNetPubSub.begin();
}

void ArtNetDataService::onDataUpdated()
{
}