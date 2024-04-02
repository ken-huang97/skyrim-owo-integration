#pragma once
#include "Sensation.h"
#include "BakedSensation.h"

namespace OWOGame
{
    class SensationsBaker {
    public:
        static sharedPtr< BakedSensation> Bake(sharedPtr< Sensation> sensation, int id, owoString name);
    };
}