#ifndef __IVIRTUALDESTROY__
#define __IVIRTUALDESTROY__

#include "IBasicInterface.h"

namespace Utils
{

    class IVirtualDestroy : IBasicInterface 
	{
	public:
	  virtual ~IVirtualDestroy() {}
	};	
	
	class IVirtualDestroyOwner : IBasicInterface 
	{
	public:
		virtual void Add(IVirtualDestroy*) = 0;
		virtual void Delete(IVirtualDestroy*) = 0;
	};	
	
}  // namespace Utils

#endif

