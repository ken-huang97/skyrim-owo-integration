#pragma once

#include "Sensation.h"
#include "Exceptions.h"

namespace OWOGame
{
    class SensationsParser {
    public:
        static uniquePtr<OWOGame::Sensation> Parse(owoString value, int priority = 0);
    };
}