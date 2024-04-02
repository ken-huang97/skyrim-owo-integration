#pragma once
#include "../Domain/OWOTypes.h"
#include "Math.h"
#include <string>

namespace OWOGame
{
    class Muscle {
    private:
        int position;
        int intensity;

    public:
        Muscle(int position, int intensity = 100) : position(position), intensity(intensity) {}

        owoString ToString();

        Muscle WithIntensity(int newIntensity);

        static Muscle Pectoral_R();
        static Muscle Pectoral_L();
        static Muscle Abdominal_R();
        static Muscle Abdominal_L();
        static Muscle Arm_R();
        static Muscle Arm_L();
        static Muscle Dorsal_R();
        static Muscle Dorsal_L();
        static Muscle Lumbar_R();
        static Muscle Lumbar_L();
    };
}