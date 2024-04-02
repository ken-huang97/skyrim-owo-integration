#pragma once
#include <string>
#include "Network.h"
#include "SecretKeysVault.h"

namespace OWOGame
{
    class NotifyPresence
    {
    private:
        Network* network;
        ConnectionCandidatesCollection* keys;

        const owoString PRESENCE_MESSAGE = "ping";

    public:

        NotifyPresence(Network* network, ConnectionCandidatesCollection* keys) :
            network(network), keys(keys) {}

        void Execute(owoString addressee);
    };
}