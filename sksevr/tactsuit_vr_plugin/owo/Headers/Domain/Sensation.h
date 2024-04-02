#pragma once
#include <string>
#include <vector>
#include "../Domain/OWOTypes.h"
#include "Muscle.h"

namespace OWOGame
{
    class Sensation {
    private:
        int priority = 0;
    public:
        int GetPriority() { return priority; };
        void SetPriority(int newPriority) { priority = newPriority; };

        virtual  sharedPtr<Sensation> WithMuscles(owoVector<Muscle> muscles) = 0;
        virtual float TotalDuration() = 0;
        virtual sharedPtr<Sensation> Clone() = 0;

        virtual owoString ToString() = 0;
        virtual ~Sensation() = default;
    };
}