#include "stdafx.h"

#include "BoardDs2155.h"
#include "Bert.h"
#include "BitUtils.h"


namespace Ds2155
{

	Bert::Bert(IMembersToBoard &board):
		m_board(board),
		m_isTesting(false),
		m_lPerChannelData(0),
		m_sendInv(true),
		m_recvInv(true),
		m_direction(LINE),
		m_aIS(false),
		m_pcpr(0)
    {

    }
    
   	//--------------------------------------------	

	Bert::~Bert()
    {
    	ESS_ASSERT(!m_isTesting && !m_aIS && "Bert Testing & AIS must be stoped");
    }
    
   	//--------------------------------------------	
   	
   	   	
	void Bert::Start(bool direction)
	{		
		m_board.isDevicePresent();					
		m_isTesting = true;
		m_direction = direction;
		SetOptions();
		StartRxBertTest();
		StartTxBertTest();
		m_bertErrors = 0;		
	}
    
   	//--------------------------------------------	
	
	void Bert::Stop()
	{
		m_board.isDevicePresent();		
		m_isTesting = false;		
		StopRxBertTest();
		StopTxBertTest();
	}	
    
   	//--------------------------------------------
   	
	bool Bert::isTesting() const
	{
		return m_isTesting;
	}
	
    //---------------------------------------------------------------
    
    void Bert::EnableAIS()
    {
    	m_aIS = true;
    	m_board.MaskClear(L_LIC2, 0x10);        	
	}
    
    //---------------------------------------------------------------
    
    void Bert::DisableAIS()
    {
    	m_aIS = false;        	
		m_board.MaskClear(L_LIC2, 0x10);        	
		m_board.MaskSet(L_LIC2, 0x10);            
    }
        
    //----------------------------------------------------------    
    
	bool Bert::isAISGen() const
	{
		return m_aIS;
	}	

	//------------------------------------------------
		
	void Bert::OneErrInsert()
	{
		
		m_board.isDevicePresent();		
		m_board.MaskClear(L_BC2, 0x10);      				
		m_board.MaskSet(L_BC2, 0x10);      							
		
	}
	
	//------------------------------------------------		
	
	void Bert::ErrCounterUpd()
	{
				
		m_board.Write(L_BC1, ((m_board.Read(L_BC1) & 0xfc) | 0x02 ));
		m_board.Write(L_BC1, ((m_board.Read(L_BC1) & 0xfc) | 0x00 ));									
		
		dword res = m_board.Read(L_BEC1); 
		res |= m_board.Read(L_BEC2)<<8;
		res |= m_board.Read(L_BEC3)<<16;		
		
		m_bertErrors += res; 		
	}
	
	//------------------------------------------------	
	
	dword Bert::UpdateErrBitCount()
	{				
		m_board.isDevicePresent();		
		ErrCounterUpd();
		return m_bertErrors;
	}
	
   	//--------------------------------------------		
	
	void Bert::ClearErrBitCount()
	{
		m_bertErrors = 0;
	}
					
   	//--------------------------------------------	
   	   		
	void Bert::SetOptions()
	{   
		if(m_direction) 
		{
			m_board.MaskSet(L_BIC, 0x02);
			m_board.Write(L_LBCR, 0x01);			
		}
		else
		{

			m_board.MaskClear(L_BIC, 0x02);
		}

		SendInv(m_sendInv);
		RecvInv(m_recvInv);
		
   		if (m_aIS) DisableAIS();
		m_board.MaskClear(L_BC1, 0x1c);      	
		m_board.MaskSet(L_BC1, 0x04);
	}
		
	//------------------------------------------------

	void Bert::SendInv(bool inv)
	{
		m_board.MaskClear(L_BC1, 0x40);
   		if (inv) m_board.MaskSet(L_BC1, 0x40);			
	}

	//------------------------------------------------

	void Bert::RecvInv(bool inv)
	{
		m_board.MaskClear(L_BC1, 0x20);
   		if (inv) m_board.MaskSet(L_BC1, 0x20);			
	}
	
	//---------------------------------------------------
	 
	void Bert::StartRxBertTest()
	{  
		
		SetBit(m_pcpr, 4);
		m_board.Write(L_PCPR, 0x11);
		m_board.Write(L_PCDR1, 0);
		m_board.Write(L_PCDR2, 0);
		m_board.Write(L_PCDR3, 0);
		m_board.Write(L_PCDR4, 0);

		m_board.Write(L_PCPR, m_pcpr);
		SetChannelsMask();

		m_board.MaskSet(L_BIC, 0x01);
		m_board.Write(L_SR9, 0xfd);
		m_board.Read(L_SR9);

		m_board.Write(L_BC1, ((m_board.Read(L_BC1) & 0xfc) | 0x01 ));
		m_board.Write(L_BC1, ((m_board.Read(L_BC1) & 0xfc) | 0x80 ));
		m_board.Write(L_BC1, ((m_board.Read(L_BC1) & 0xfc) | 0x02 ));
			
	} 				
	
	//---------------------------------------------------		
	 
	void Bert::StopRxBertTest()
	{
		ClearBit(m_pcpr, 4);			
		m_board.Write(L_PCPR, 0x11);
		ClearChannelsMask();
		m_board.Write(L_PCPR, m_pcpr);
		SetChannelsMask();
		byte temp = m_board.Read(L_PCPR);		
		if(!TestBit(temp, 0x0)) m_board.MaskClear(L_BIC, 0x01);      	
	} 		
	
	//---------------------------------------------------

	void Bert::SetChannelsMask()
	{

		byte temp = (byte)m_lPerChannelData;
   		m_board.Write(L_PCDR1, temp);		
		temp = (byte)(m_lPerChannelData>>8);
   		m_board.Write(L_PCDR2, temp);		
		temp = (byte)(m_lPerChannelData>>16);
   		m_board.Write(L_PCDR3, temp);		
		temp = (byte)(m_lPerChannelData>>24);
   		m_board.Write(L_PCDR4, temp);		   		   				
	}
	
	//---------------------------------------------------

	void Bert::ClearChannelsMask()
	{

   		m_board.Write(L_PCDR1, 0);
   		m_board.Write(L_PCDR2, 0);   		
   		m_board.Write(L_PCDR3, 0);   		
   		m_board.Write(L_PCDR4, 0);   		
   		
   	}	

	//------------------------------------------------

	void Bert::StartTxBertTest()
	{
		ESS_ASSERT(m_lPerChannelData && "No TimeSlots for BERT");
   		   		   		
		SetBit(m_pcpr, 0);						
		m_board.Write(L_PCPR, 0x11);
		ClearChannelsMask();
		m_board.Write(L_PCPR, m_pcpr);
		SetChannelsMask();
		
		m_board.MaskClear(L_BC1, 0x80);      				
		m_board.MaskSet(L_BC1, 0x80);      							
		m_board.MaskSet(L_BIC, 0x01);
       
	}   		   	
   	
	//------------------------------------------------

	void Bert::StopTxBertTest()
	{

		ClearBit(m_pcpr, 0);  			
		m_board.Write(L_PCPR, 0x11);
		ClearChannelsMask();
		m_board.Write(L_PCPR, m_pcpr);
		SetChannelsMask();

		m_board.MaskClear(L_BC1, 0x80);      				
		m_board.MaskSet(L_BC1, 0x80);      															
		
		m_board.Write(L_LBCR, 0x00);		
		
		byte temp = m_board.Read(L_PCPR);
		if(!TestBit(temp, 0x4)) m_board.MaskClear(L_BIC, 0x01);      				                
	}   		
	
	//------------------------------------------------		
	
    void Bert::SetChannelToTest(byte ch)	// подключить канал тестированию		
    {
    	ESS_ASSERT(ch < m_board.GetChannelsCount() && "Illegal channel number");
		m_board.isDevicePresent();    	
		m_lPerChannelData |= (dword)(1 << ch);		
    }
               
	//------------------------------------------------		        
	
    void Bert::ClearAllChannels()		// отключить все каналы от тестироавния	
	{		
		m_board.isDevicePresent();		
		m_lPerChannelData =0;
	}
};
