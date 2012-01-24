#ifndef __CRITICALREGION__
#define __CRITICALREGION__

namespace iVDK
{

    // PC CriticalRegion -- does nothing
	class CriticalRegion : boost::noncopyable
	{
    public:
        CriticalRegion()
        {
            // nothing 
        }
        
        ~CriticalRegion()
        {
            // nothing             
        }
	};
	
}  // namespace iVDK

#endif

