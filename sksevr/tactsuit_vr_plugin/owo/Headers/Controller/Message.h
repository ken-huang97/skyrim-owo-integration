#pragma once
#include <string>
#include "../Domain/OWOTypes.h"

namespace OWOGame
{
    struct Message
    {
    public:
        const owoString value;
        const owoString addressee;

        Message(owoString value, owoString addressee) : value(value), addressee(addressee) {}

        static Message* Empty() { return CreateNew(Message((owoString)"", (owoString)"")); }
    };
}