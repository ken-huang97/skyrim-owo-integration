#pragma once
#include "Sensation.h"
#include "OWOTypes.h"
#include "SensationWithMuscles.h"

namespace OWOGame
{
    class BakedSensation : public Sensation {
    public:
        int id;
        sharedPtr<Sensation> reference;
        owoString name;
        owoString icon;
        owoString family;

        BakedSensation(int id, owoString name, sharedPtr<Sensation> reference, owoString icon = "0", owoString family = "");

        sharedPtr< OWOGame::BakedSensation> WithIcon(owoString newIcon);
        sharedPtr<Sensation> WithMuscles(owoVector<Muscle> muscles);
        owoString Stringify();
        float TotalDuration();
        sharedPtr<Sensation> Clone();

        virtual owoString ToString() override;
    };
}