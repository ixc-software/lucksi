#ifndef __TDM_USER_BLOCK_H__
#define __TDM_USER_BLOCK_H__

#include "Utils/ErrorsSubsystem.h"
#include "IUserBlockInterface.h"


namespace BfTdm
{                       
    
    using namespace Platform;
    
        class UserBlock  : 
            boost::noncopyable,
        	public IUserBlockInterface
        {    	    

        // IUserBlockInterface impl
        public:

        
            // Warning!!! if using T1 Mode, then  0 <= timeSlot <= 23;
            void CopyRxTSBlockTo(byte timeSlot, byte *tsBuff, word offset, word tsSampleCount);
            void CopyRxTSBlockTo(byte timeSlot, word *tsBuff, word offset, word tsSampleCount);
            void PutTxTSBlockFrom(byte timeSlot, const byte *tsBuff, word offset, word tsSampleCount);
            void PutTxTSBlockFrom(byte timeSlot, const word *tsBuff, word offset, word tsSampleCount);
            void CopyTxTSBlockTo(byte timeSlot, byte *tsBuff, word buffSize);  
            void CopySyncTxTo(byte timeSlot, byte *tsBuff,  word offset, word tsSampleCount);
	        void CopySyncTxTo(byte timeSlot, word *tsBuff,  word offset, word tsSampleCount);
            
            // Warning!!! if using T1 Mode, then frame consist of 32-th channels,
            // but channels 0,4,8,12,16,20,24,28 are ignored,
            // for correct data adressing use ChannelCalc() method
            
            const word *GetRxBlock(word frameNum) const;
            word *GetTxBlock(word frameNum) const;

            void UserFree();                                                
            word GetTSBlockSize() const;                                    

        public:    	                

            UserBlock(word tsblockSize, word timeSlotsCount, word *txBuff, word *rxBuff, bool T1Mode, bool byteDataMode);
            word *GetTxBuffPointer() const;
            word *GetRxBuffPointer() const;              

            // only if not collision 
            void SetUserUsing();          

            bool isUserUsing() const { return m_usedByUser; }  // for debug only

            void DetectCollision()
            {
                if (m_usedByUser) m_collision = true;
            }
            
            bool ResetCollision()
            {
                bool ret = m_collision;
                m_collision = false;
                return ret;
            }

            static byte ChannelCalc(byte t1ts);
            
            
        private:        

            template<class T> void CopyRxTSBlock(byte timeSlot, T *tsBuff, word offset, word tsSampleCount);        
            template<class T> void PutTxTSBlock(byte timeSlot, const T *tsBuff, word offset, word tsSampleCount);
	        template<class T> void CopySyncTx(byte timeSlot, T *tsBuff,  word offset, word tsSampleCount);            
        
                   
            word* m_TXpBuff;
            word* m_RXpBuff;        
            bool  m_usedByUser;
            word  m_TSblockSize;
            word  m_timeSlotsCount;
            bool  m_T1Mode;
            bool  m_byteDataMode;            
            bool  m_txBuffModified;
            bool  m_collision;
        };                
    
} //namespace BFTdm

#endif

