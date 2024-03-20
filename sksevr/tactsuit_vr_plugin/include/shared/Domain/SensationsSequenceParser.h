#pragma once
#include "SensationsSequence.h"
#include "String.h"
#include "SensationsParser.h"

namespace OWOGame
{
    class SensationsSequenceParser {
    public:
        static bool CanParse(owoString value);
        static uniquePtr<OWOGame::SensationsSequence> Parse(owoString value);
    };
}