#ifndef __DS2155_HDLC_PROFILE__
#define __DS2155_HDLC_PROFILE__

#include "stdafx.h"
#include "Platform/PlatformTypes.h"

namespace Ds2155
{
    using namespace Platform;

    /*
       Hdlc memory pool & bidirBuff profile
		.....
    */

	class HdlcProfile 
	{
	public:
	    
	    HdlcProfile(
	    	int memPoolBlockSize,
			int blocksNum,		
			int bidirOffset,
			int maxPackSize):
	    	m_memPoolBlockSize(memPoolBlockSize),
			m_blocksNum(blocksNum),		
			m_bidirOffset(bidirOffset),
			m_maxPackSize(maxPackSize)
		{
		};			
		
		int GetMemPoolBlockSize()const
		{
			return m_memPoolBlockSize;
		};
		
		int GetBlocksNum()const
		{
			return m_blocksNum;
		};
		
		int GetBidirOffset()const
		{
			return m_bidirOffset;
		};
		
		int GetMaxPackSize()const
		{
			return m_maxPackSize;
		};		
			
	private: 							

    	int m_memPoolBlockSize;
		int m_blocksNum;	
		int m_bidirOffset;
		int m_maxPackSize;		

	};

};

#endif
