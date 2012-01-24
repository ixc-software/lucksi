#ifndef __VDKTHREADDCB__
#define __VDKTHREADDCB__

#include "Platform/Platform.h"
#include "Utils/FixedFifo.h"
#include "Utils/VirtualInvoke.h"
#include "Utils/MemoryPool.h"
#include "Utils/ErrorsSubsystem.h"

namespace BfDev
{
	
	class VdkThreadDcb : boost::noncopyable
	{


	public:
		
		VdkThreadDcb(int queueSize, int invokeBlockSize = 64) 
		{
		}

        ~VdkThreadDcb()
        {
        }

        // called from IRQ context!
        // use for -- void TBase::fn(T param)
        template<class TFn, class TBase, class TParam>
        void PutMsg(TFn fn, TBase &base, TParam &param)
        {
            using namespace Utils;

            typedef Utils::VIMethodWithParam<TBase, TParam> VI;

            boost::scoped_ptr<IVirtualInvoke> vi( new VI(fn, base, param) );
            vi->Execute();
        }
       	
	};

		
} // namespace BfDev

#endif
