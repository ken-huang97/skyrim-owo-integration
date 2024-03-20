// Copyright (C) 2023 OWO Games - All Rights Reserved
#pragma once
#include "Network.h"
#include "NotifyAbscense.h"
#include "../Domain/OWOTypes.h"
#include "SendAuthMessage.h"
#include "ScanAvailableServers.h"
#include <chrono>
#include <thread>

namespace OWOGame
{
    class FindServer
    {
    private:
        Network* network;
        NotifyPresence* notifyPresence;
        NotifyAbscense* notifyAbscense;
        SendAuthMessage* sendAuth;
        ConnectionCandidatesCollection* candidates;

        const owoString VERIFICATION_RESPONSE = "pong";
        const owoString PRESENCE_NOTIFICATION = "okay";
        owoString lastResponse = "";
        owoString lastServer = "";
        owoVector<owoString> addressee;
        owoString auth = "";
        owoString gameId = "";

        void Connect();
        void ListenForDisconnection();
        bool Contains(owoVector<owoString> input, owoString token);
        bool IsUniqueConnection();
        void UpdateStatus(owoString currentAddressee);

    public:

        FindServer(Network* network, NotifyPresence* scanAvailableServers,
            NotifyAbscense* notifyAbscense, SendAuthMessage* sendAuth,
            ConnectionCandidatesCollection* keys) :
            network(network), notifyPresence(scanAvailableServers),
            notifyAbscense(notifyAbscense), sendAuth(sendAuth), candidates(keys) {}

        ~FindServer();

        void Initialize(owoVector<owoString> newAddressee, owoString newAuth, owoString newGameId);
        void Execute();
        void Scan();
    };
}