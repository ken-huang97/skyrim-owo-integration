#pragma once
#include <string>
#include <vector>
#include "../Domain/OWOTypes.h"
#include "BakedSensation.h"

namespace OWOGame
{
    class GameAuth {
    public:
        owoString id;
        owoVector <owoString> sensations;

        GameAuth(owoVector<owoString> sensations, owoString id)
            : sensations(sensations), id(id) {}

        static sharedPtr<GameAuth> Create(owoVector<owoString> sensations, owoString id = "0");
        static sharedPtr<GameAuth> Parse(owoString value, owoString id = "0");

        owoString ToString();
    };
}