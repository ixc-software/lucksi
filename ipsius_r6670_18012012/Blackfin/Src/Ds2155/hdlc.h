#ifndef __DS2155_HDLC__
#define __DS2155_HDLC__

#include "Platform/platformtypes.h"
#include "utils/IBasicInterface.h"
#include "Utils/BidirBufferCreator.h"
#include "Utils/FixedFifo.h"
#include "ds2155_reg.h"
#include "hdlcregmap.h"
#include "hdlcprofile.h"
#include "HdlcLogProfile.h"
#include "ILogHdlc.h"
#include "HdlcStats.h"

namespace Ds2155
{
    using namespace Platform;
    using Utils::IBidirBuffCreator;
    using Utils::BidirBuffer;            
    
	class IMembersToBoard;
	class Hdlc;    

    //--------------------------------------------------------------------	    
        
    /*
    	��������� Board -> Hdlc
    */
        
    class IBoardToHdlc : public Utils::IBasicInterface
    {
    	
    public:
    
    	virtual	void Polling(bool txOvrun) = 0;
		virtual	int GetChannelLink() const = 0;    	
    	
    };
    
    //--------------------------------------------------------------------	        

    /*
		����� HDLC �����������        
    */
    
        
	class Hdlc  : boost::noncopyable,
	 	public IBoardToHdlc
	{
	public:
	    			 
		Hdlc(IMembersToBoard &board,
			 byte ctrlNum, 
			 int maxPackSize,
			 Utils::IBidirBuffCreator &buffCreator,
			 int poolBlocksNum,
			 byte channel,
			 ILogHdlc *log,
			 HdlcLogProfile logProfile);
			 			 
		~Hdlc();								
		
		bool isRxPack() const;
		BidirBuffer *GetPack();		//0 ���� ������� �����
		int GetTxPackCount() const;
		int GetRxPackCount() const;		
		
				
		ErrorFlags PeekErrors();		
		const HdlcStatistic &PeekStatistic() const;
				
		bool SendPack(void *ptr, int size);						
		

	//IBoardToHdlc impl
	private:						
	
    	void Polling(bool txOvrun);	
		int GetChannelLink() const;    	
    	
	private:					
	
		void TransmitFSM();
		void ReadBuff(byte count);
		void TxReset();
		void RxReset();
		void Reset();				
		void SetWaterMarks(WaterMark txwm, WaterMark rxwm);		
		
		void OnPack();
		void OnBadPack();
		void OnRcvIgnPack();
		void OnTxAbort();
		void OnSendComplete();
		void DelRxBidirBuff();
		void DelTxBidirBuff();
		
		void Log(const std::string str, bool addTag = false);

		
		
	private:	
	
		IMembersToBoard &m_board;
        IBidirBuffCreator &m_buffCreator;
		HdlcRegMap   m_regMap;				

        Utils::FixedFifo<BidirBuffer*> m_txBlocksFifo;
        Utils::FixedFifo<BidirBuffer*> m_rxBlocksFifo;        
		
		
		BidirBuffer *m_currTxBuffer;
		BidirBuffer *m_currRxBuffer;		
		
		HdlcLogProfile m_logProfile;
		ILogHdlc *m_log;
		
		int  m_channel;    				    // ����� ������ ��������
		bool m_isRTS;  					    // Ready To Send, ���������� ����� � ��������
		bool m_isBadPack;					// ���� - ������� ����� �����		
		int m_maxPackSize;					// 
		int m_maxPollingDelay;					// 		
		dword m_lastTime;					// 				
		bool m_firstPolling;
		bool m_firstFSM;		
		int m_maxFSMTime;					// 				
		
		ErrorFlags 	m_errSruct;				// ��������� ������ ������
		HdlcStatistic	m_statistic;			// ��������� �������� ����������
		
		std::string	m_logStr;
		
		bool m_test;
				
	};

    //--------------------------------------------------------------------	        

};

#endif







