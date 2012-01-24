#ifndef _BF_ADI_BUFFER_HELPER_H_
#define _BF_ADI_BUFFER_HELPER_H_

#include "stdafx.h"
#include <drivers/adi_dev.h>

namespace BfDev
{
	template<class T> struct AdiBufferHelper{};

	template<> struct AdiBufferHelper<ADI_DEV_1D_BUFFER>
	{
		typedef ADI_DEV_1D_BUFFER Type;
		static const ADI_DEV_BUFFER_TYPE Id = ADI_DEV_1D;
	};

	template<> struct AdiBufferHelper<ADI_DEV_2D_BUFFER>
	{
		typedef ADI_DEV_2D_BUFFER Type;
		static const ADI_DEV_BUFFER_TYPE Id = ADI_DEV_2D;
	};
	
	template<> struct AdiBufferHelper<ADI_DEV_CIRCULAR_BUFFER>
	{
		typedef ADI_DEV_CIRCULAR_BUFFER Type;
		static const ADI_DEV_BUFFER_TYPE Id = ADI_DEV_CIRC;		
	};


	template<> struct AdiBufferHelper<ADI_DEV_SEQ_1D_BUFFER>
	{
		typedef ADI_DEV_SEQ_1D_BUFFER Type;
		static const ADI_DEV_BUFFER_TYPE Id = ADI_DEV_SEQ_1D;				
	};

};

#endif
