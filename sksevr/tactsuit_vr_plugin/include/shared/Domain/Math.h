#pragma once
#include "OWOTypes.h"

namespace OWOGame
{
    class Math
    {
    public:
        template<typename T>
        static T Clamp(T value, T min, T max) { return owoclamp(value, min, max); }
    };
}