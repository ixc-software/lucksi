#ifndef ALLOC_INTF_H
#define ALLOC_INTF_H

#include "Utils/IBasicInterface.h"

namespace ISDN
{

	class IIsdnAlloc : public Utils::IBasicInterface
	{
	public:
		virtual void * alloc(size_t size) = 0;
		virtual void free(void* ptr) = 0;
        //virtual ~IIsdnAlloc() {};
	};

};

#endif
