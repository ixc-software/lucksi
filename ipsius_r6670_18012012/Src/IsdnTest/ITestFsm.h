#ifndef ITESTFSM_H
#define ITESTFSM_H

// #include "IsdnTestConfig.h"
#include "ITestState.h"
#include "Platform/PlatformTypes.h"

namespace IsdnTest
{
	using Platform::dword;

	class ITestFsm
	{
	public:

		virtual void SetStartTest(ITestState* start) = 0;		

		virtual void SwitchTo(ITestState* pNewState) = 0;

		virtual dword GetTick() = 0;

        virtual ~ITestFsm(){};
	};
};

#endif
