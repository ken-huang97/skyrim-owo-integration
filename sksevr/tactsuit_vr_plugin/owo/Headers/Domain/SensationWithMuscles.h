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

        sharedPtr<Sensation> WithMuscles(owoVector<Muscle> newMuscles);
        float TotalDuration();
        sharedPtr<Sensation> Clone();

        virtual owoString ToString() override;
    };
}