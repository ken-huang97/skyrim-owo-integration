#pragma once
#include "Microsensation.h"
#include "../Domain/OWOTypes.h"
#include "Exceptions.h"
#include "String.h"

namespace OWOGame
{
    class MicroSensationParser {
    public:
        static sharedPtr< OWOGame::MicroSensation> Parse(owoString value);
    };
}