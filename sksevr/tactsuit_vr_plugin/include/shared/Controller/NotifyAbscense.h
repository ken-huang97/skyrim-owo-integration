#pragma once
#include "Network.h"
#include "SecretKeysVault.h"

namespace OWOGame
{
    class NotifyAbscense
    {
    private:
        Network* network;
        ConnectionCandidatesCollection* keys;

    public:
        NotifyAbscense(Network* network, ConnectionCandidatesCollection* keys) :
            network(network), keys(keys) {}

        void Execute(owoString gameId);
    };
}