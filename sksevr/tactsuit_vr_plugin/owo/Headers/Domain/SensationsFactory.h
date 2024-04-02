#pragma once
#include "Microsensation.h"
#include "SensationsSequence.h"

namespace OWOGame
{
    class SensationsFactory {
    public:
        static sharedPtr< MicroSensation> Create(int frequency = 100, float duration = 0.1, int intensity = 100,
            float rampUp = 0, float rampDown = 0, float exitTime = 0, owoString name = "", int priority = 0);
        static sharedPtr< SensationsSequence> CreateSequence(owoVector<sharedPtr< Sensation>> sensations, int priority = 0);
    };
}