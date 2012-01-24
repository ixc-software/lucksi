#ifndef __EXENAME__
#define __EXENAME__

#include "stdafx.h"

namespace Utils
{
	
    class ExeName
    {
    public:

        static void Init(const char *pExeName);
        static const std::string& GetExeName();
        static const std::string& GetExeDir();  // ends with "/"

    };
	
}  // namespace Utils
    

#endif

