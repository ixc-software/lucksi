#include "stdafx.h"

#include "Utils/MemLeak.h"

#include "AssertTest.h"
#include "AdiGPTimer.h"

// -----------------------------

namespace
{
	
	void TimerHandle(int &arg)
	{
		ESS_ASSERT(0 && "From IRQ context");		
	}
	
	void RunIrqAssert()
	{
		BfDev::AdiGPTimer t(ADI_TMR_GP_TIMER_0);
		
		int i = 0;
		t.Start(TimerHandle, i, 1);
	}
	
	void RunKernelPanic()
	{
		Utils::MemLeak::LeakForever();
	}
	
}  // namespace


// -----------------------------

namespace BfDev
{
	
	void RunAssertTest()
	{
		// usual
		// ESS_ASSERT(0 && "From main context");
		
		// from IRQ
		// RunIrqAssert();
		
		// kernel panic
		RunKernelPanic();
	}

	
} // namespace BfDev
