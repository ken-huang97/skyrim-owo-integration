#pragma once
#include "Sensation.h"
#include "BakedSensation.h"

namespace OWOGame
{
    class SensationsBaker {
    public:
        static uniquePtr< BakedSensation> Bake(uniquePtr< Sensation> sensation, int id, owoString name);
    };
}