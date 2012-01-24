#include "stdafx.h"
#include "Platform/platformtypes.h"
#include "RecursiveThrow.h"

// --------------------------------------

namespace
{
    class T;

    void Fn1(T &t) {}

    void Fn2()
    {
        T *p;
        // Fn1(p);  // T& vs T* test
    }

}

// --------------------------------------

namespace BfPort
{

	namespace Detail
	{
        const int CStackBuffSize = 32;
		
        int ExecuteAtRecurseLevel(int level, IExecutive &exe)
        {
            if (level <= 0)
            {
                exe.Execute();
                return 0;
            }

            Platform::byte buff[CStackBuffSize];
            int sum = 0;

            for(int i = 0; i < CStackBuffSize; ++i)
            {
                buff[i] = level;
                sum += buff[i];
            }

            return sum + ExecuteAtRecurseLevel(level - 1, exe);
        }
		
	}  // namespace Detail


		
}  // namespace BfPort

