#pragma once
#include "Message.h"
#include <map>

namespace OWOGame
{
    class ConnectionCandidatesCollection
    {
    private:
        owoSet<owoString> appCandidates;
    public:

        void Store(owoString message);
        bool ContainsCandidate(owoString candidate);
        owoVector<owoString> GetCandidates();
        void Clear();
    };
}