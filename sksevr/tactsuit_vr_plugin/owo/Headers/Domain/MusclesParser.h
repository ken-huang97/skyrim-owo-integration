#pragma once
#include "../Domain/OWOTypes.h"
#include "MusclesGroup.h"

namespace OWOGame
{
    class MusclesParser {
    public:
        static MusclesGroup Parse(owoString value);
    };
}