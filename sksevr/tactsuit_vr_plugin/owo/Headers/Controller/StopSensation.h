#pragma once

#include "Client.h"
#include "../Domain/GameAuth.h"

namespace OWOGame
{
    class StopSensation
    {
    private:
        Client* client;
        sharedPtr<GameAuth> game = nullptr;

    public:
        StopSensation(Client* client);

        void Execute();
        void Configure(sharedPtr<GameAuth> newGame);
    };
}