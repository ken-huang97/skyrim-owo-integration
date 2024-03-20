#pragma once
#include "Sensation.h"
#include "OWOTypes.h"
#include "SensationWithMuscles.h"

namespace OWOGame
{
    class BakedSensation : public Sensation {
    public:
        int id;
        uniquePtr<Sensation> reference;
        owoString name;
        owoString icon;
        owoString family;

        BakedSensation(int id, owoString name, uniquePtr<Sensation> reference, owoString icon = "0", owoString family = "");

        uniquePtr< OWOGame::BakedSensation> WithIcon(owoString newIcon);
        uniquePtr<Sensation> WithMuscles(owoVector<Muscle> muscles);
        owoString Stringify();
        float TotalDuration();
        uniquePtr<Sensation> Clone();

        virtual owoString ToString() override;
    };
}