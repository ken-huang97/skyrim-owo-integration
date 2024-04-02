#pragma once
#include "../Domain/Sensation.h"
#include "../Domain/GameAuth.h"
#include "Client.h"

namespace OWOGame
{
    class SendSensation
    {
    private:
        Client* client = nullptr;
        sharedPtr<GameAuth> game = nullptr;
        int lastPriority = -1;
        uint64_t whenPreviousSensationEnds = 0;

        bool IsPlaying(uint64_t timeSinceStart);
    public:

        SendSensation(Client* client);

        void Execute(sharedPtr<OWOGame::Sensation> sensation, uint64_t millisecondsSinceStart = 0);
        void ResetPriority();
        void Configure(sharedPtr<GameAuth> newGame);
    };
}