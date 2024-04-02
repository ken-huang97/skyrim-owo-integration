#pragma once
#include "SensationWithMuscles.h"
#include "String.h"
#include "SensationsParser.h"
#include "MusclesParser.h"

namespace OWOGame
{
    class SensationWithMusclesParser {
    public:
        static bool CanParse(owoString value);
        static sharedPtr<OWOGame::SensationWithMuscles> Parse(owoString value);
    };
}