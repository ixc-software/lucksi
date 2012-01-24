#ifndef __CRITICALREGION__
#define __CRITICALREGION__

namespace iVDK
{

    // RAII for VDK CriticalRegion
	class CriticalRegion : boost::noncopyable
	{
    public:
        CriticalRegion();
        ~CriticalRegion();
        
        static void Enter();
        static void Leave();
	};
	
}  // namespace iVDK

#endif

