#pragma once
#include <string>
#include "Network.h"
#include "SecretKeysVault.h"

namespace OWOGame
{
    class SendAuthMessage
    {
    private:
        Network* network;
        ConnectionCandidatesCollection* keys;

    public:

        SendAuthMessage(Network* network, ConnectionCandidatesCollection* keys) :
            network(network), keys(keys) {}

        void Execute(owoString auth, owoString addressee);
    };
}