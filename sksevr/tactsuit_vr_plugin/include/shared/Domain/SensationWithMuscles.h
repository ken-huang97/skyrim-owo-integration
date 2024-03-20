#pragma once
#include "Sensation.h"
#include "Muscle.h"
#include "MusclesGroup.h"
#include <vector>
#include "BakedSensation.h"

namespace OWOGame
{
    class SensationWithMuscles : public Sensation {
    public:
        sharedPtr<OWOGame::Sensation> reference;
        MusclesGroup muscles;

        SensationWithMuscles(sharedPtr<OWOGame::Sensation> reference, MusclesGroup muscles);

        uniquePtr<Sensation> WithMuscles(owoVector<Muscle> newMuscles);
        float TotalDuration();
        uniquePtr<Sensation> Clone();

        virtual owoString ToString() override;
    };
}