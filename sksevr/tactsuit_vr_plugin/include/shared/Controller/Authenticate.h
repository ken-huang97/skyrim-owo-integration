#pragma once
#include <string>
#include "../Domain/GameAuth.h"
#include "../Domain/OWOTypes.h"
#include "../Controller/Client.h"

namespace OWOGame
{
    class Authenticate
    {
    private:
        Client* client;
        sharedPtr<GameAuth> game = nullptr;

        owoString AuthMessage();

    public:

        Authenticate(Client* client);

        void Configure(sharedPtr<GameAuth> game);
        OWOGame::ConnectionState ManualConnect(owoVector<owoString> ip);
        OWOGame::ConnectionState AutoConnect();
    };
}