#include "stdafx.h"

#include <ccblkfn.h>
#include "UserChBlockList.h"
#include "UserBlock.h"
    
namespace BfTdm
{
    using namespace Platform;

    
    //-----------------------------------------------------------                        
    
    UserChBlockList::UserChBlockList():
    	m_allocAllow(true)
    {                       
    }    
                
    //---------------------------------------------------------------------                                
    
    UserChBlockList::~UserChBlockList()
    {                       
        UserBlockList::iterator i = m_blockList.begin();
        
        for (; i != m_blockList.end(); ++i)
            delete (*i);                        
    }

    //-----------------------------------------------------------                    
    
    UserBlock &UserChBlockList::GetCursorBlock()
    {
    	m_allocAllow = false;
    	return *(*m_userCursor);
    }
    
    //-----------------------------------------------------------                
    
    void UserChBlockList::IncCursor()
    {
    	m_allocAllow = false;    	
        m_userCursor++;
        if (m_userCursor == m_blockList.end()) m_userCursor = m_blockList.begin();        
    }
    
    //-----------------------------------------------------------            
                
    void UserChBlockList::AllocUserBlock(
        word userBlockSize,
        word timeSlotsCount,
        word *txBuff,
        word *rxBuff,
        bool T1Mode,
        bool byteDataMode)
    {
   		ESS_ASSERT(m_allocAllow == true && "UserBlock iterator using before AllocBlock");				    	
        UserBlock *p_userBlock = new UserBlock(userBlockSize, timeSlotsCount, txBuff, rxBuff, T1Mode, byteDataMode);
        m_blockList.push_back(p_userBlock);
        m_userCursor = m_blockList.begin();        
    }
    
    //-----------------------------------------------------------                
    
    bool UserChBlockList::CheckUsersBlocksFree()
    {
    	UserBlockList::iterator userBlockIterator;
    	bool allfree = true;
    	
    	for (userBlockIterator = m_blockList.begin(); userBlockIterator != m_blockList.end(); userBlockIterator++)
    	{
    		if((*userBlockIterator)->isUserUsing()) allfree = false;
    	}
		return allfree;
    }
    
    //-----------------------------------------------------------                
    
    int UserChBlockList::GetUsedUsersBlocksCount()
    {
    	UserBlockList::iterator userBlockIterator;
    	int result = 0;
    	
    	for (userBlockIterator = m_blockList.begin(); userBlockIterator != m_blockList.end(); userBlockIterator++)
    	{
    		if((*userBlockIterator)->isUserUsing()) result++;    		
    	}
		return result;    	
    }
    
    //-----------------------------------------------------------            
    
                     
} //namespace BFTdm
