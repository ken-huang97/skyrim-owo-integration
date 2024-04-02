#pragma once
#include "Network.h"
#include "SecretKeysVault.h"

namespace OWOGame
{
    class SendEncryptedMessage
    {
    private:
        Network* network;

    public:

        SendEncryptedMessage(Network* network) :
            network(network) {}

        void Execute(owoString message);
    };
}