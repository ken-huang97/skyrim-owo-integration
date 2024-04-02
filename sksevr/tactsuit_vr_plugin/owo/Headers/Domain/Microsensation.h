#pragma once
#include "Math.h"
#include "Sensation.h"
#include "../Domain/OWOTypes.h"
#include "BakedSensation.h"
#include "SensationWithMuscles.h"

namespace OWOGame
{
    class MicroSensation : public Sensation {
    public:
        const int frequency;
        const float duration;
        const int intensity;
        const float rampUp;
        const float rampDown;
        const float exitTime;
        const owoString name;

        MicroSensation(int frequency, float duration, int intensity, float rampUp, float rampDown, float exitTime, owoString name);

        float TotalDuration();

        sharedPtr<Sensation> WithMuscles(owoVector<Muscle> muscles);
        sharedPtr<Sensation> Clone();

        virtual owoString ToString() override;
    };
}