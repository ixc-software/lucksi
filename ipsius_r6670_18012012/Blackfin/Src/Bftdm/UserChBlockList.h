#ifndef __TDM_USER_CH_BLOCK_LIST_H__
#define __TDM_USER_CH_BLOCK_LIST_H__

#include "Platform/Platform.h"
#include "UserBlock.h"

namespace BfTdm
{                       
    using namespace Platform;                  
    
    class UserChBlockList  : boost::noncopyable
    {    	                    
    
    public:    	            
    
    	UserChBlockList();        
        ~UserChBlockList();        
        
        UserBlock &GetCursorBlock();
        void IncCursor();
        bool CheckUsersBlocksFree();
		int GetUsedUsersBlocksCount();
        
        void AllocUserBlock(
	        word userBlockSize,
	        word timeSlotsCount,
	        word *txBuff,
	        word *rxBuff,
	        bool T1Mode,
	        bool byteDataMode);
        
                        
    private:        	    
        
        typedef std::vector<UserBlock*>  UserBlockList;
        UserBlockList    m_blockList;
        
        UserBlockList::iterator m_userCursor;        
        bool m_allocAllow;
        
    };
		
} //namespace BFTdm

#endif

