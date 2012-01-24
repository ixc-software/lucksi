#ifndef ITSTATE_H
#define ITSTATE_H

// #include "IsdnTestConfig.h"

namespace IsdnTest
{
	class ITestState
	{
	public:

		virtual void Process() = 0;
		virtual std::string GetName() = 0;
        virtual ~ITestState(){};

	};
}//namespace IsdnTest

#endif
