#pragma once
#include "../Domain/GameAuth.h"
#include "Authenticate.h"
#include "SendSensation.h"
#include "StopSensation.h"
#include "../ClientFactory.h"
#include "../Infraestructure/UDPNetwork.h"

namespace OWOGame {

    class OWO
    {
    private:
        Client* client;
        Authenticate connect;
        SendSensation send;
        StopSensation stop;
        uint64_t lastUpdate = 0;
        uint64_t lastUpdateScan = 0;
        uint64_t timeInMs = 0;
        uint64_t UPDATE_FREQUENCY = 500;

    public:
        OWO(Client* client);
        ~OWO();

        void Configure(sharedPtr<GameAuth> auth);
        void Send(uniquePtr<OWOGame::Sensation> sensation);
        void Stop();
        OWOGame::ConnectionState UpdateStatus(uint64_t timeInMs);
        OWOGame::ConnectionState AutoConnect();
        OWOGame::ConnectionState Connect(owoVector<owoString> ip);
        owoVector<owoString> DiscoveredApps();
        void Scan(uint64_t timeInMs);
        void Disconnect();
        ConnectionState State();

        void ChangeUpdateFrequency(uint64_t newFrequency);

        static sharedPtr<OWO> Create()
        {
            return CreateNewUnique(OWO, OWO(ClientFactory::Create(CreateNew(UDPNetwork))));
        };
    };
}