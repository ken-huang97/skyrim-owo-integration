#pragma once
#include <string>
#include <vector>
#include "../Domain/OWOTypes.h"
#include "Muscle.h"


namespace OWOGame
{
    class MusclesGroup {
    private:
        owoVector<Muscle> muscles;

    public:
        MusclesGroup(owoVector<Muscle> muscles) : muscles(muscles) {}

        MusclesGroup WithIntensity(int intensity);
        owoString ToString();
        operator owoVector<Muscle>() { return muscles; }

        static MusclesGroup Front();
        static MusclesGroup Back();
        static MusclesGroup All();
    };
}