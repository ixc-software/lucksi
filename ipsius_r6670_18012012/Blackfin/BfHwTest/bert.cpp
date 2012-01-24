#include "Ds2155access.h"
#include "Bert.h"
#include "BitUtils.h"
#include "UartSimple.h"

bool	m_isTesting = false;	// статус тестирования
dword   m_lPerChannelData = 0;	// маска каналов для потока тестирования
bool	m_sendInv = true;		// инверсия потока передачи		
bool	m_recvInv =true;		// инверсия потока приема
bool	m_direction = LINE;		// направление генерации потока
bool	m_aIS = false;			// состояние генерации аварии AIS
byte	m_pcpr = 0;				// настроечный регистр
dword	m_bertErrors;			// счетсик ошибок в потоке тестирования				

      
   	//--------------------------------------------
   	
	bool BertisTesting()
	{
		return m_isTesting;
	}
	
    //---------------------------------------------------------------
    
    void BertEnableAIS()
    {
    	m_aIS = true;
    	DSMaskClear(L_LIC2, 0x10);        	
	}
    
    //---------------------------------------------------------------
    
    void BertDisableAIS()
    {
    	m_aIS = false;        	
		DSMaskClear(L_LIC2, 0x10);        	
		DSMaskSet(L_LIC2, 0x10);            
    }
        
    //----------------------------------------------------------    
    
	bool BertisAISGen()
	{
		return m_aIS;
	}	

	//------------------------------------------------
		
	void BertOneErrInsert()
	{						
		DSMaskClear(L_BC2, 0x10);      				
		DSMaskSet(L_BC2, 0x10);      									
	}
	
	//------------------------------------------------		
	
	void BertErrCounterUpd()
	{
				
		DSWrite(L_BC1, ((DSRead(L_BC1) & 0xfc) | 0x02 ));
		DSWrite(L_BC1, ((DSRead(L_BC1) & 0xfc) | 0x00 ));									
		
		dword res = DSRead(L_BEC1); 
		res |= DSRead(L_BEC2)<<8;
		res |= DSRead(L_BEC3)<<16;		
		
		m_bertErrors += res; 		
	}
	
	//------------------------------------------------	
	
	dword BertUpdateErrBitCount()
	{				
				
		BertErrCounterUpd();
		BertErrCounterUpd();		
		return m_bertErrors;
	}
	
   	//--------------------------------------------		
	
	void BertClearErrBitCount()
	{
		BertErrCounterUpd();		
		m_bertErrors = 0;
	}

	//------------------------------------------------

	void BertSendInv(bool inv)
	{
		DSMaskClear(L_BC1, 0x40);
   		if (inv) DSMaskSet(L_BC1, 0x40);			
	}

	//------------------------------------------------

	void BertRecvInv(bool inv)
	{
		DSMaskClear(L_BC1, 0x20);
   		if (inv) DSMaskSet(L_BC1, 0x20);			
	}	
						
   	//--------------------------------------------	
   	   		
	void BertSetOptions()
	{   
		if(m_direction) 
		{
			DSMaskSet(L_BIC, 0x02);
//			DSWrite(L_LBCR, 0x01);			
		}
		else
		{
			DSMaskClear(L_BIC, 0x02);
		}

		BertSendInv(m_sendInv);
		BertRecvInv(m_recvInv);
		
   		if (m_aIS) BertDisableAIS();
		DSMaskClear(L_BC1, 0x1c);      	
		DSMaskSet(L_BC1, 0x04);
	}

	//---------------------------------------------------

	void BertSetChannelsMask()
	{
		byte temp = (byte)m_lPerChannelData;
   		DSWrite(L_PCDR1, temp);		
		temp = (byte)(m_lPerChannelData>>8);
   		DSWrite(L_PCDR2, temp);		
		temp = (byte)(m_lPerChannelData>>16);
   		DSWrite(L_PCDR3, temp);		
		temp = (byte)(m_lPerChannelData>>24);
   		DSWrite(L_PCDR4, temp);		   		   				
	}	
			
	//---------------------------------------------------
	 
	void BertStartRxBertTest()
	{  		
		SetBit(m_pcpr, 4);
		DSWrite(L_PCPR, 0x11);
		DSWrite(L_PCDR1, 0);
		DSWrite(L_PCDR2, 0);
		DSWrite(L_PCDR3, 0);
		DSWrite(L_PCDR4, 0);

		DSWrite(L_PCPR, m_pcpr);
		BertSetChannelsMask();

		DSMaskSet(L_BIC, 0x01);
		DSWrite(L_SR9, 0xfd);
		DSRead(L_SR9);

		DSWrite(L_BC1, ((DSRead(L_BC1) & 0xfc) | 0x01 ));
		DSWrite(L_BC1, ((DSRead(L_BC1) & 0xfc) | 0x80 ));
		DSWrite(L_BC1, ((DSRead(L_BC1) & 0xfc) | 0x02 ));
			
	} 				
	
	//---------------------------------------------------

	void BertClearChannelsMask()
	{
   		DSWrite(L_PCDR1, 0);
   		DSWrite(L_PCDR2, 0);   		
   		DSWrite(L_PCDR3, 0);   		
   		DSWrite(L_PCDR4, 0);   		   	
   	}		
	
	//---------------------------------------------------		
	 
	void BertStopRxBertTest()
	{
		ClearBit(m_pcpr, 4);			
		DSWrite(L_PCPR, 0x11);
		BertClearChannelsMask();
		DSWrite(L_PCPR, m_pcpr);
		BertSetChannelsMask();
		byte temp = DSRead(L_PCPR);		
		if(!TestBit(temp, 0x0)) DSMaskClear(L_BIC, 0x01);      	
	} 		
	
	//------------------------------------------------

	void BertStartTxBertTest()
	{   		   		   		
		SetBit(m_pcpr, 0);						
		DSWrite(L_PCPR, 0x11);
		BertClearChannelsMask();
		DSWrite(L_PCPR, m_pcpr);
		BertSetChannelsMask();
		
		DSMaskClear(L_BC1, 0x80);      				
		DSMaskSet(L_BC1, 0x80);      							
		DSMaskSet(L_BIC, 0x01);
       
	}   		   	
   	
	//------------------------------------------------

	void BertStopTxBertTest()
	{

		ClearBit(m_pcpr, 0);  			
		DSWrite(L_PCPR, 0x11);
		BertClearChannelsMask();
		DSWrite(L_PCPR, m_pcpr);
		BertSetChannelsMask();

		DSMaskClear(L_BC1, 0x80);      				
		DSMaskSet(L_BC1, 0x80);      															
		
		DSWrite(L_LBCR, 0x00);		
		
		byte temp = DSRead(L_PCPR);
		if(!TestBit(temp, 0x4)) DSMaskClear(L_BIC, 0x01);      				                
	}   		
	
	//------------------------------------------------		
	
    void BertSetChannelToTest(byte ch)	// подключить канал тестированию		
    {		    	
		m_lPerChannelData |= (dword)(1 << ch);		
    }
               
	//------------------------------------------------		        
	
    void BertClearAllChannels()		// отключить все каналы от тестироавния	
	{						
		m_lPerChannelData =0;
	}

   	//--------------------------------------------	

	void BertStart(bool direction)
	{		
		m_isTesting = true;
		m_direction = direction;
		BertSetOptions();
		BertStartRxBertTest();
		BertStartTxBertTest();
		m_bertErrors = 0;		
	}

   	//--------------------------------------------	
	
	void BertStop()
	{
		m_isTesting = false;		
		BertStopRxBertTest();
		BertStopTxBertTest();
	}	
	
   	//--------------------------------------------	
	bool last_lss = true;
   	
   	
	bool BertLSS()
	{
		bool lss;
		DSWrite(L_SR9, 0xFF);
		byte sr9 = DSRead(L_SR9);		
		
		if (sr9 & 0x2)
		{
			lss = true;
	        if(!last_lss && lss) Send0Msg("SR9, Bit 2");		 
		}
        else lss = false;
        		          		 
      	if(!TestBit(DSRead(L_BIC), 0)) 
      	{
      		lss = true;
			Send0Msg("L_BIC bit 0");						      		
      	}

		if (((DSRead(L_BC1) >> 2) & 0x7) != 0x4)
		{
		 if (TestBit(sr9 ,3)) lss = true;
		 if (TestBit(sr9 ,2)) lss = true;
		 if (!(DSRead(L_INFO2) & 0x80)) lss = true;
		}    	    			
		return lss;
	}	
	
