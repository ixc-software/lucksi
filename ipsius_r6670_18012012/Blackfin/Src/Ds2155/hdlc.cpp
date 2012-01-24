#include "stdafx.h"

#include "BoardDs2155.h"
#include "Hdlc.h"
#include <cdefBF537.h>



namespace Ds2155
{		
	
	Hdlc::Hdlc(IMembersToBoard &board, 
		byte ctrlNum, 
		int maxPackSize,
		IBidirBuffCreator &buffCreator,
		int poolBlocksNum,
		byte channel,
		ILogHdlc *log,
		HdlcLogProfile logProfile) :
		m_board(board),
		m_buffCreator(buffCreator),
		m_regMap(ctrlNum),
		m_currTxBuffer(0),
		m_currRxBuffer(0),
		m_rxBlocksFifo(poolBlocksNum),
		m_txBlocksFifo(poolBlocksNum),
		m_maxPackSize(maxPackSize),
		m_log(log),
		m_logProfile(logProfile),
		m_channel(channel),
		m_isRTS(true),
		m_test(true),
		m_isBadPack(false),
		m_lastTime(0),
		m_firstPolling(true),
		m_maxPollingDelay(0),
		m_maxFSMTime(),
		m_firstFSM(true),
		m_logStr("")
    {    			
    	ESS_ASSERT(m_channel && "Channel 0 is not alowed for HDLC");
    	ESS_ASSERT(m_channel < m_board.GetChannelsCount() && "This channel is not alowed for HDLC");
  		
		m_board.Write(m_regMap.HTC, 0x08);          
		m_board.Write(m_regMap.HRC, 0);            

		SetWaterMarks(WM80, WM64);
		
  		dword mask = 1<<m_channel;
  		    
		m_board.Write(m_regMap.HRCS1, (byte)mask);
		m_board.Write(m_regMap.HRCS2, (byte)(mask>>8));
		m_board.Write(m_regMap.HRCS3, (byte)(mask>>16));
		m_board.Write(m_regMap.HRCS4, (byte)(mask>>24));

		m_board.Write(m_regMap.HTCS1, (byte)mask);
		m_board.Write(m_regMap.HTCS2, (byte)(mask>>8));
		m_board.Write(m_regMap.HTCS3, (byte)(mask>>16));
		m_board.Write(m_regMap.HTCS4, (byte)(mask>>24));		
		
		m_board.Register(this, ctrlNum);
		Reset();
    	    	
    }
    
   	//--------------------------------------------	

	Hdlc::~Hdlc()
    {
		Log(m_statistic.ToString());    	

		std::ostringstream log;
		log << "      RxFiFoSize = " << m_rxBlocksFifo.Size() << std::endl;			
		log << "      TxFiFoSize = " << m_txBlocksFifo.Size() << std::endl;					
		Log(log.str());
		
		if (m_currTxBuffer) Log("Tx Pack Aborted, destructor\n");
		if (m_currRxBuffer) Log("Rx Pack Aborted, destructor\n");		
		if (!m_isRTS) Log("Tx Pack Sended, waiting for Ack, destructor\n");		
		
		    	
    	Reset();
    	m_txBlocksFifo.Clear();
    	m_rxBlocksFifo.Clear();    	    	
		DelTxBidirBuff();
		DelRxBidirBuff();
		m_board.Unregister(this);
    }
    
	// ----------------------------------------------------------------
	
	void Hdlc::RxReset()
	{

		m_board.MaskSet(m_regMap.HRC, 0x80);
		m_board.MaskClear(m_regMap.HRC, 0x80);		
		
	}
	
	// ----------------------------------------------------------------
	
	void Hdlc::TxReset()
	{
		
		m_board.MaskSet(m_regMap.HTC, 0x20);
		m_board.MaskClear(m_regMap.HTC, 0x20);		
		
	}
	
	// ----------------------------------------------------------------
	
	void Hdlc::Reset()
	{
	
		RxReset();			
		TxReset();	
		
	}
    
	// ----------------------------------------------------------------
	
	void Hdlc::SetWaterMarks(WaterMark txwm, WaterMark rxwm)
	{
		byte mask = rxwm;
		mask |= txwm<<3;
		m_board.Write(m_regMap.HFC, mask);		
	}	
	
	// ----------------------------------------------------------------

	void Hdlc::TransmitFSM()
	{  
		
		
		if (!m_currTxBuffer)
		{
			if (m_isRTS)
			{								
				m_currTxBuffer = m_txBlocksFifo.Pop();
				m_isRTS = false;
				if (m_logProfile.m_isTxFSMEnabled)
					Log("    TxQueue--\n");				
				m_logStr = "";					
				if (m_logProfile.m_isTransivedTransActSize)
					m_logStr += "----------\n";
			}
			else return;
		}
		
	
		byte capacity = m_board.Read(m_regMap.HTFBA);			
		
		if (m_logProfile.m_isTransivedTransActSize)
		{
			std::ostringstream log;
			log << "      Tx cap = " << (int)capacity << std::endl;			
			m_logStr += log.str();
		}
				
				
		for (byte i=0; i<capacity-1; i++)	
		{
			if (m_currTxBuffer->Size()==1)
			{
				m_board.MaskSet(m_regMap.HTC, 0x04);				
  				m_board.Write(m_regMap.HTF, m_currTxBuffer->PopFront());
				capacity = m_board.Read(m_regMap.HTFBA);
				DelTxBidirBuff();
				if (m_logProfile.m_isTxFSMEnabled)
					Log("    Tx EOP\n");				
				if (m_logProfile.m_isTransivedTransActSize)
					m_logStr += "    Tx EOP\n";					
	 			return;
				
			}
			else			
  				m_board.Write(m_regMap.HTF, m_currTxBuffer->PopFront());  			  			
		}
		
		if (m_logProfile.m_isTxFSMEnabled)
		{
			std::ostringstream log;
			log << "      Tx cap = " << (int)capacity << std::endl;			
			Log(log.str());
		}
		
	}

	// ----------------------------------------------------------------
	
	bool Hdlc::SendPack(void *ptr, int size)
	{
				
		ESS_ASSERT(size > 1 && "Too small packet lenght");
		ESS_ASSERT(size <= m_maxPackSize && "Too big packet");
		
		if (m_txBlocksFifo.IsFull())
		{
			m_errSruct.txFiFoFool = true;
			m_statistic.txFifoFullCount++;
			if (m_logProfile.m_isTxUserLevelEnabled)
				Log("    txFiFoFool\n");
			return false;
		}

		m_board.isDevicePresent();
		
		BidirBuffer *newBuff;
		
        try
        {
			newBuff = m_buffCreator.CreateBidirBuff();
        }
        catch(/*const*/ Utils::NoBlocksAvailable &e)
        {
			m_statistic.txAllocErrorsCount++;
			m_errSruct.txAllocErrorFlag = true;
			if (m_logProfile.m_isTxUserLevelEnabled)
				Log("    No BidirBuffer free \n");			
			return false;
        }		

        		
        if (!newBuff)
        {
			if (m_logProfile.m_isTxFSMEnabled)
				Log("    No BidirBuffer free \n");			    	
				
	        return false;
        }

                
		newBuff->PushBack(ptr, size);
		
		m_txBlocksFifo.Push(newBuff);
		
		
		if(m_logProfile.m_isTxUserLevelEnabled)
			Log("Send pack\n", true);		

/*			
				Log("       Dump: ");
				std::ostringstream dump;
				for(int i=0; i < newBuff->Size(); ++i)
					dump << " 0x" << std::hex << (int)(*newBuff)[i];				
					dump << std::endl;

				Log(dump.str());								
*/
								
		if (m_logProfile.m_isTxFSMEnabled)
		{			
			std::ostringstream log;
			log << "    TxQueue++, Len = " << newBuff->Size();	
			Log(log.str());			
			if (m_logProfile.m_isTxDumpPack)
			{
				Log("       Dump: ");
				std::ostringstream dump;
				for(int i=0; i < newBuff->Size(); ++i)
					dump << " 0x" << std::hex << (int)(*newBuff)[i];				
					dump << std::endl;

				Log(dump.str());								
			}
		}			
		
		return true;
	}
	
	//---------------------------------------------
	void Hdlc::ReadBuff(byte count)
	{

	    if (!count) return;

	    	        
		bool is_RME = false;

		if(!(count & 0x80)) is_RME = true;
		
		count &= 0x7f;	    
		
		if (m_logProfile.m_isTransivedTransActSize)
		{
			std::ostringstream log;
			log << "      RxRD = " << (int)count <<
			",  RME = " << is_RME << std::endl;			
			m_logStr += log.str();
		}		
		
		
		if (m_logProfile.m_isRxFSMEnabled)
		{
			std::ostringstream log;
			log << "      RxRD = " << (int)count <<
			",  RME = " << is_RME << std::endl;			
			Log(log.str());
		}		
		
						
		//проверка на продолжение приема битого пакета  
		if (!m_isBadPack)  
		{  
			
			if ((m_currRxBuffer->Size() + count) <= m_maxPackSize)
			{
				for (word i=0; i<count; i++)
					m_currRxBuffer->PushBack(m_board.Read(m_regMap.HRF));
			}
			else
			{
				for (word i=0; i<count; i++)
					(*m_currRxBuffer)[0] = m_board.Read(m_regMap.HRF);          
					
	        	m_isBadPack = true;				
				if (m_logProfile.m_isRxFSMEnabled)
					Log("    Bad pack flag, packet is too big\n");
	        	
			}			
    	
	    	if (is_RME)
	    	{
				byte rstatus = m_board.ReadStatus(m_regMap.INFH, 0x3f) & 0x07;

	      		if (!rstatus) return;			//если is_RME ошибочный
      		
	      		//если данных больше чем максимальная длина пакета или размера входного буфера недостаточно      
	      		if (m_isBadPack) rstatus = 0x06; // выдаем event о битом пакете с ошибкой 5;
				if (m_logProfile.m_isTransivedTransActSize)
				{
					std::ostringstream log;
					log << "rstatus = " << (int)rstatus << std::endl;			
					m_logStr += log.str();
				}		

	      		if (rstatus == 1) OnPack(); // CRC OK	
	      		if (rstatus > 1)   // выдаем event о битом пакете с кодом ошибки
	      		{
					OnBadPack();
					RxReset();
	        		return;          
	      		}      
	    	}
		}
	  	else 
	  	{
		    //если пакет уже помечен как битый или нет буфера
	    
		    //считываем бесполезные данные из буфера
		    byte rstatus=0;
		    
		    for (word i=0; i<count; i++) rstatus += m_board.Read(m_regMap.HRF);
		    
		    //если конец битого пакета, то выдаем event об игнорированном пакете
		    if (is_RME)
		    {
				rstatus = m_board.ReadStatus(m_regMap.INFH, 0x3f);
	      		rstatus &= 0x07;        
	      		if (rstatus)
	      		{
	        		m_isBadPack = false;          
	        		OnRcvIgnPack();
					RxReset();
	      		}
	    	}
	  	}
	}	
	
   	//--------------------------------------------	        
    
	void Hdlc::Polling(bool txOvrun)
	{		
		if(m_firstPolling)	
		{
			m_firstPolling = false;
		}
		else
		{
			dword time = GetSystemTickCount();
			
			if (time - m_lastTime > m_maxPollingDelay)
			{			
				m_maxPollingDelay = time - m_lastTime;
				std::ostringstream log;
				log << "maxPollingDelay =" << (int)m_maxPollingDelay << std::endl;
				Log(log.str());				
				
			}						
			m_lastTime = GetSystemTickCount();					
		}
					
		byte status;
		
		status = m_board.ReadStatus(m_regMap.SRH, 0x7f);


		if (m_logProfile.m_isTransivedTransActSize && !m_isRTS)
		{
			std::ostringstream log;
			log << "       TxPs = " << std::hex << (int)(status & 0x43) << std::endl;
			if(txOvrun)				
				log << "       TxOVR " << std::endl;				
			m_logStr += log.str();
		}

		if(m_logProfile.m_isTxPollingEnabled && !m_isRTS)
		{
			std::ostringstream log;
			log << "       TxPs = " << std::hex << (int)(status & 0x43) << std::endl;
			Log(log.str());
		}			

		
			if (!m_isRTS && txOvrun) 
		{
				m_isRTS = true;
				OnTxAbort();
		}
					
		if (status & 0x02) 
			if(!m_txBlocksFifo.IsEmpty() || m_currTxBuffer) 
			{
				TransmitFSM();
			}
	
		if (status & 0x40)
		{
			m_isRTS = true;         
			OnSendComplete();
		}								


	    byte count = m_board.Read(m_regMap.HRPBA);
		if (!(count & 0x80) || status & 0x08)
		{
			
			if (m_logProfile.m_isTransivedTransActSize && m_currRxBuffer)
			{
				std::ostringstream log;
				log << "       RxPs = 0x" << std::hex << (int)(status & 0x2C) << std::endl;
				m_logStr += log.str();
			}

			if(m_logProfile.m_isRxPollingEnabled)
			{
				std::ostringstream log;
				log << "\n       RxPs = 0x" << std::hex << (int)(status & 0x2C) << std::endl;
				Log(log.str());
			}			
														
			if (!m_currRxBuffer && !m_isBadPack)
			{					
				m_logStr = "";					
	            try
	            {
					m_currRxBuffer = m_buffCreator.CreateBidirBuff();
	            }
	            catch(/*const*/ Utils::NoBlocksAvailable &e)
	            {
					m_isBadPack = true;
					m_currRxBuffer = 0;
					m_statistic.rxAllocErrorsCount++;
					m_errSruct.rxAllocErrorFlag = true;
					if (m_logProfile.m_isRxUserLevelEnabled)
						Log("    Bad pack flag, No space for packet\n");
	            }
	            
				if (m_logProfile.m_isRxFSMEnabled)
					Log("    RxNewpack\n");			
			}		
		    byte count = m_board.Read(m_regMap.HRPBA);      							
			ReadBuff(count);
		}      				
		
			
		if(m_firstFSM)
		{
			m_firstFSM = false;			
		}
		else
		{
			dword time = GetSystemTickCount();
			
			if (time - m_lastTime > m_maxFSMTime)
			{			
				m_maxFSMTime = time - m_lastTime;
				std::ostringstream log;
				log << "maxFSMTime =" << (int)m_maxFSMTime << std::endl;
				Log(log.str());				
				
			}						
		}
		m_lastTime = GetSystemTickCount();					
	}

   	//--------------------------------------------	    
   	
	BidirBuffer *Hdlc::GetPack()
	{
		m_board.isDevicePresent();		
		if (m_rxBlocksFifo.Size())
		{
			if (m_logProfile.m_isRxFSMEnabled)		
				Log("    RxQueue--");
				
			
			return m_rxBlocksFifo.Pop();
		}
		else return 0;
	}					
	
   	//--------------------------------------------	    
   	
	bool Hdlc::isRxPack() const
	{
		m_board.isDevicePresent();		
		return !m_rxBlocksFifo.IsEmpty();
	}		
		
   	//--------------------------------------------	    
   	
	int Hdlc::GetChannelLink() const
	{
				
		return m_channel;
		
	}	

   	//--------------------------------------------	    	   	
	
	void Hdlc::OnPack()
	{
		if(m_logProfile.m_isRxUserLevelEnabled)
			Log("Pack recieved \n", true);
		
		if (m_logProfile.m_isRxFSMEnabled)
			Log("    RxQueue++\n");
		
		if(m_logProfile.m_isRxDumpPack)			
		{						
			Log("      rX Dump: ");
			std::ostringstream dump;
			for(int i=0; i<m_currRxBuffer->Size(); ++i)
				dump << " 0x" << std::hex << (int)(*m_currRxBuffer)[i];				
				dump << std::endl;

			Log(dump.str());
		}					

		
		if(m_test)
		{
			m_test = false;
				std::ostringstream log;
				log << "      ID = 0x " << std::hex << (int)m_board.Read(L_IDR) << std::endl;							
				log << "      Rx Size = " << (int)m_currRxBuffer->Size() << std::endl;			
				m_logStr += log.str();
				m_logStr += "\n";
				
				Log(m_logStr);			
			
		}
				
		if (m_logProfile.m_isTransivedTransActSize)
		{
			bool sizeErr = (m_currRxBuffer->Size() != 3);
			byte pack[3] = {0,1,0x7f};

			bool contentErr = false;
		
			for(int i=0; i<m_currRxBuffer->Size(); ++i)
			{
				if ((*m_currRxBuffer)[i] != pack[i])
				{
					contentErr = true;
					std::ostringstream log;
					log << "      Bad Content byte Num = " << i << std::endl;
					m_logStr += log.str();
					break;					
				}
			}
			
			if (contentErr || sizeErr)
			{
				std::ostringstream log;
				log << "      Rx Size = " << (int)m_currRxBuffer->Size() << std::endl;			
				m_logStr += log.str();
				m_logStr += "\n";
														
				Log(m_logStr);			
			}			
		}

		if(m_rxBlocksFifo.IsFull())
		{
			m_errSruct.rxFiFoFool = true;
			m_statistic.rxFifoFullCount++;
			if (m_logProfile.m_isRxUserLevelEnabled)
				Log("    rxFiFoFool\n");			
			DelRxBidirBuff();
		}
		else
		{
			m_rxBlocksFifo.Push(m_currRxBuffer);
			m_statistic.packRecieved++;			
			m_currRxBuffer = 0;			
		}
	}
	
		
   	//--------------------------------------------	    	   	
	
	void Hdlc::OnBadPack()
	{
		m_statistic.rxBadPackCount++;
		m_errSruct.rxBadPackFlag = true;
		if (m_logProfile.m_isRxUserLevelEnabled)
			Log("    Bad packet recieved, CRC fail\n");		
		DelRxBidirBuff();
	}
	
   	//--------------------------------------------	    		

	void Hdlc::OnRcvIgnPack()
	{
		m_statistic.rxIgnoredPackCount++;		
		m_errSruct.rxIgnoredPackFlag = true;
		if (m_logProfile.m_isRxUserLevelEnabled)
			Log("    Ignored packet recieved\n");				
		DelRxBidirBuff();
	}
	
   	//--------------------------------------------	    		
	
	void Hdlc::OnTxAbort()
	{
		m_statistic.txOverRuned++;				
		m_errSruct.txOverRunedFlag = true;
		if (m_logProfile.m_isTxUserLevelEnabled)
			Log("    Send aborted, overrun\n");				
		DelTxBidirBuff();		
	}
	
   	//--------------------------------------------	    			
	
	void Hdlc::OnSendComplete()
	{
		m_statistic.packSended++;

		if (m_logProfile.m_isTransivedTransActSize)
			m_logStr += "    Tx SCmpl\n";

				
		if (m_logProfile.m_isTxFSMEnabled)
			Log("    Tx SCmpl\n");		
		if(m_logProfile.m_isTxUserLevelEnabled)
			Log("Send Completed \n", true);
	}
	
   	//--------------------------------------------	    			
	
	void Hdlc::DelRxBidirBuff()
	{
		if (m_currRxBuffer) delete m_currRxBuffer;
		m_currRxBuffer = 0;						
	}
	
   	//--------------------------------------------	    			
	
	void Hdlc::DelTxBidirBuff()
	{
		if (m_currTxBuffer) delete m_currTxBuffer;
		m_currTxBuffer = 0;						
	}	

   	//--------------------------------------------	    	
	
	ErrorFlags Hdlc::PeekErrors()
	{
		m_board.isDevicePresent();	
		ErrorFlags errStruct;
		
		errStruct = m_errSruct;
		m_errSruct.Clear();
		
		return errStruct;				
	}
	
	//------------------------------------------------		
		
	int Hdlc::GetTxPackCount() const
	{
		m_board.isDevicePresent();		
		return m_txBlocksFifo.Size();
	}	
	
	//------------------------------------------------		
		
	int Hdlc::GetRxPackCount() const
	{
		m_board.isDevicePresent();		
		return m_rxBlocksFifo.Size();
	}		
	
	//------------------------------------------------		
			
	const HdlcStatistic &Hdlc::PeekStatistic() const
	{
		m_board.isDevicePresent();		
		return m_statistic;
	}
	
   	//--------------------------------------------	    		
   	
	void Hdlc::Log(const std::string str, bool addTag)
	{
		if (!m_log) return;
//		if (addTag)	m_log->Add("HDLC: " + str);
//		else 		m_log->Add(str);
		m_log->Add(str);
	}
	
   	//--------------------------------------------	    			
			
};
