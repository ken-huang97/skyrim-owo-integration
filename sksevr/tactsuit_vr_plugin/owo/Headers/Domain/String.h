#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include "../Domain/OWOTypes.h"
#include <algorithm>
#include <iterator>
#include <vector>

namespace OWOGame
{
    class String {
    public:
        static owoVector<owoString> Split(owoString value, char delim)
        {
            owoVector<owoString> result;

            owoString aux = "";
            for (int i = 0; i < value.size(); i++) {
                if (value[i] == delim) {
                    result.push_back(aux);
                    aux = "";
                }
                else {
                    aux += value[i];
                }
            }

            if(aux != "")
                result.push_back(aux);

            return result;
        }

        static bool Contains(owoString value, char character)
        {
            return value.find(character) != owoStringNpos;
        }
    };
}