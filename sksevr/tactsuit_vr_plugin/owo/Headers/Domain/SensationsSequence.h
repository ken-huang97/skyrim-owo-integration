#pragma once
#include <vector>
#include "Sensation.h"
#include "SensationWithMuscles.h"
#include "BakedSensation.h"

namespace OWOGame
{
    class SensationsSequence : public Sensation
    {
    private:
        owoVector<sharedPtr<Sensation>> sensations;
    public:
        SensationsSequence(owoVector<sharedPtr<Sensation>> sensations);

        sharedPtr<Sensation> WithMuscles(owoVector<Muscle> muscles);
        float TotalDuration();
        sharedPtr<Sensation> Clone();

        ~SensationsSequence() {}
        virtual owoString ToString() override;
    };
}