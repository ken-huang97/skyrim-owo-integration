// Copyright (C) 2023 OWO Games - All Rights Reserved
#pragma once
#include "Network.h"
#include "SendEncryptedMessage.h"
#include "../Domain/OWOTypes.h"
#include "FindServer.h"
#include "../Domain/Exceptions.h"
#include <thread>

namespace OWOGame
{
    class Client
    {
    private:
        Network* network;
        SendEncryptedMessage* sendMessage;
        FindServer* findServer;
        ConnectionCandidatesCollection* candidates;

    public:
        Client(Network* network, SendEncryptedMessage* sendMessage, FindServer* findServer, ConnectionCandidatesCollection* candidates) :
            network(network), sendMessage(sendMessage), findServer(findServer), candidates(candidates) {}

        ~Client();

        void StartConnection(owoVector<owoString> addressee, owoString auth, owoString gameId);
        void UpdateStatus();
        void Send(owoString message);
        void Disconnect();
        void Scan();
        owoVector<owoString> DiscoveredApps();
        bool IsConnected();
        ConnectionState State();
    };
}