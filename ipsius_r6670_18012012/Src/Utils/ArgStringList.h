#ifndef ARGSTRINGLIST_H
#define ARGSTRINGLIST_H

#include "stdafx.h"

namespace Utils
{
    class ArgStringList : public std::vector<std::string>
    {
    public:
        ArgStringList() {}

        ArgStringList(int argc, char* argv[])
        {
            for(int i = 0; i < argc; ++i)
            {
                push_back(argv[i]);
            }
        }
    };

} // namespace Utils

#endif
