// Copyright (C) 2023 OWO Games - All Rights Reserved
#pragma once
#include "./Controller/Client.h"

namespace OWOGame
{
    class ClientFactory
    {
    public:
        static Client* Create(Network* network);
    };
}