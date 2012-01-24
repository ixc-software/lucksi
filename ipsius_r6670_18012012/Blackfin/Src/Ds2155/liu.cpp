#include "stdafx.h"

#include "BoardDs2155.h"
#include "Liu.h"
#include "BitUtils.h"


namespace Ds2155
{

	
	Liu::Liu(IMembersToBoard &board, const LiuProfile &src):
	    m_board(board),
	    m_profile(src),
	    m_lPcpr(0),	
		m_lTestInterfInit(0),
        m_stateLeds(0),
		m_ledsCondition(0),
		m_yellowAlarm(false)
    {
    }
	
    
   	//--------------------------------------------
	
    std::string &Liu::ToString(std::string &str) const
	{
		using std::cout;
		using std::endl;
		
		str += "Profile: ";
		str += m_profile.ToString();
		str += "\n";
		
		return str;
	}

    //--------------------------------------------------------------------		
	
    bool Liu::Init()
    {        
        if(m_board.isT1Mode()) return InitT1();
        return InitE1();
    }
			            
    //--------------------------------------------------------------------	
	//Функциия инициализации чипа	    

    bool Liu::InitE1()
    {
        if (!isDS2155Present()) return false;
        
		*pPORTFIO_INEN		|= 0x8000;
        *pPORTF_FER	|= 0x8020;
		*pTIMER4_CONFIG		= 0x00AD;
		*pTIMER4_PERIOD		= 256;
		*pTIMER4_WIDTH		= 1;
		*pTIMER_ENABLE		|= 0x0010;        
    
        m_board.Write(L_MSTRREG, 0x3);
        m_board.Write(L_MSTRREG, 0x2);
        
        m_board.Write(L_IOCR1,   0); // RSYNC is an output 
        m_board.Write(L_IOCR2,   0x3);
        
        m_board.Write(L_CCR1,    0x4);
        m_board.Write(L_E1RCR1,  0x24);
        m_board.Write(L_E1TCR1,  0x54);
        m_board.Write(L_ESCR, 0x1);


       //LIU
        m_board.Write(L_LIC1, 0x21);
        m_board.Write(L_LIC2, 0x90);
        m_board.Write(L_LIC3, 0x0);
        m_board.Write(L_LIC4, 0x0f);
        

        m_board.Write(L_TS1, 0x0b); //mfas

        for (byte i=0; i<15; i++)
        {
            m_board.Write(L_TS2 + i, 0xFF);
        }

        m_lPcpr = 0;
        m_lTestInterfInit = 0;
        m_ledsCondition = 0;

        SaInsTransmit(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);             
                
        // выставление частоты синхронизации        
        if(m_profile.SyncMaster()) m_board.Write(L_CCR1, 0x04);
        else m_board.Write(L_CCR1, 0x06);        
        
        m_board.Write(L_CCR2, 0x07);                
        
        
        
        // 1 длинная линия; 0 короткая линия;
        LongLine(0);
        
        // 0-ami; 1-hdb3;
        if (m_profile.LineCode() == Enums::lcHdb3)
        {
            //hdb3
			m_board.MaskSet(L_E1TCR1, 0x04);
			m_board.MaskSet(L_E1RCR1, 0x20);			  
        }
        else
        {
            //ami
			m_board.MaskClear(L_E1TCR1, 0x04);
			m_board.MaskClear(L_E1RCR1, 0x20);			  
        }
        // 0-pcm 30 (CAS); 1-pcm 31 (CCS); 2 -no stuct
        Cycle(m_profile.Cycle());
        

        //0 -- no CRC-4, 1 -- CRC-4,
        CRC4(m_profile.Crc4());
    
        //Установки FAS
        m_board.Write(L_TAF, m_profile.FAS());
        //Установка NFAS
        m_board.Write(L_TNAF, m_profile.NFAS());
        //Установка MFAS
        m_board.Write(L_TS1,  m_profile.MFAS());                
                
        // Сброс синхронизации
        ResetSync();
                
        return true;        
    }
    
    //--------------------------------------------------------------------	    
    
    bool Liu::InitT1()
    {
        if (!isDS2155Present()) return false;
        
		*pPORTFIO_INEN		|= 0x8000;
        *pPORTF_FER	|= 0x8020;
		*pTIMER4_CONFIG		= 0x00AD;
		*pTIMER4_PERIOD		= 256;
		*pTIMER4_WIDTH		= 1;
		*pTIMER_ENABLE		|= 0x0010;        
    	
        m_status.OperationModeE1 = false;
				
        m_board.Write(L_MSTRREG, 0x1);
        m_board.Write(L_MSTRREG, 0x0);
        
        m_board.Write(L_IOCR1,   0); // RSYNC is an output 
        m_board.Write(L_IOCR2,   0x03); // Elastic Store clock = 2048
        
        
        m_board.Write(L_T1RCR2,  0x20); // B8ZS
        m_board.Write(L_T1TCR2,  0x80); // 
                        
        m_board.Write(L_ESCR, 0x11);


       //LIU
        m_board.Write(L_LIC1, 0x21);
        m_board.Write(L_LIC2, 0x18);
        m_board.Write(L_LIC3, 0x0);
        m_board.Write(L_LIC4, 0x0f);
        

        m_lPcpr = 0;
        m_lTestInterfInit = 0;
        m_ledsCondition = 0;
      
        
        // выставление частоты синхронизации        
        if(m_profile.SyncMaster()) m_board.Write(L_CCR1, 0x04);
        else m_board.Write(L_CCR1, 0x06);        
        
        m_board.Write(L_CCR2, 0x07);                
        
        
        // 1 длинная линия; 0 короткая линия;
        LongLine(0);
        
        // Сброс синхронизации
        ResetSync();
                
        return true;        
    }
    

    //--------------------------------------------------------------

    void Liu::SaInsTransmit(byte sa8, byte sa7, byte sa6, byte sa5, byte sa4)
    {
		m_board.MaskClear(L_TSACR, 0x1F);
		m_board.MaskSet(L_TSACR, 0x1F);
        m_board.Write(L_TSa8,sa8);
        m_board.Write(L_TSa7,sa7);
        m_board.Write(L_TSa6,sa6);
        m_board.Write(L_TSa5,sa5);
        m_board.Write(L_TSa5,sa4);
    }    
    
    
    //----------------------------------------------

    void Liu::LongLine(bool mode)
    {
		m_board.MaskClear(L_LIC1, 0x10);
		if(m_board.isT1Mode()) mode = !mode;
        if (mode) m_board.MaskSet(L_LIC1, 0x10);			
    }
    
    //--------------------------------------------------------------    
    //функция установки структуры кадра (PCM30, PCM31, UnStruct)
    
    bool Liu::Cycle(Enums::CycleType mode) 
    {
        if ( mode == Enums::ctPCM30)
        {
            m_board.Write( L_PCPR, 0x0);
			m_board.MaskSet(L_E1TCR1, 0x40);
            SignalNoStruct(false);
			m_board.MaskClear(L_E1RCR1, 0x40);
            return true;
        }

        if ( mode == Enums::ctPCM31)
        {
            m_board.Write(L_PCPR, 0x0);
            SignalNoStruct(false);
            m_board.MaskSet(L_E1RCR1, 0x40);
			m_board.MaskClear(L_E1TCR1, 0x40);            
            return true;
        }
 
        if (mode == Enums::ctNoStruct)
        {
                       
            m_board.Write(L_PCPR,  0x0);
			m_board.MaskSet(L_E1RCR1, 0x40);
			m_board.MaskClear(L_E1TCR1, 0x40);
            SignalNoStruct(true);
            return true;
        }

        return false;
    }
    
    //---------------------------------------------------------------

    void Liu::SignalNoStruct(bool mode)
    {
        if(mode)
        {
			m_board.MaskClear(L_E1TCR1, 0x80);
			m_board.MaskSet(L_E1TCR1, 0x80);
            m_board.Write(L_E1TCR2,0);
            m_board.Write(L_TSACR,0);
        }
        else
        {
			m_board.MaskClear(L_E1TCR1, 0x80);
			m_board.MaskSet(L_E1TCR1, 0x00);
            
        }
    }
    
    // -------------------------------------------------------------
    
    bool Liu::CRC4(bool mode)
    {
        if (mode)
        {
            //Включение генерации CRC4 на передачу
			m_board.MaskSet(L_E1RCR1, 0x08);            
            //Включить прием CRC4
			m_board.MaskSet(L_E1TCR1, 0x01);                        
            //Генерация E бит (включается на передачу)
			m_board.MaskSet(L_E1TCR2, 0x04);                        
        }
        else
        {
			m_board.MaskClear(L_E1RCR1, 0x08);
			m_board.MaskClear(L_E1TCR1, 0x01);			        	
			m_board.MaskClear(L_E1TCR2, 0x04);			
        }
        return true;
    }

    //-----------------------------------------------------
    //функция сброса схемы синхронизации чипа
    void Liu::ResetSync()
    {
        
        if(m_board.isT1Mode())
        {
        	m_board.MaskClear(L_T1RCR1, 0x01);
    		m_board.MaskSet(L_T1RCR1, 0x01);                	
        	m_board.MaskClear(L_T1RCR1, 0x01);		                        
        	return;
        }
        
    	m_board.MaskClear(L_E1RCR1, 0x01);
		m_board.MaskSet(L_E1RCR1, 0x01);                	
    	m_board.MaskClear(L_E1RCR1, 0x01);		
                
    }        
    
    //----------------------------------------------------------
    // Boзвращает уровень принимаемого сигнала
    // 0x0 -- Уровень dB  Greater than -2.5
    // 0x1 -- Уровень dB -2.5 to -5.0
    // 0x2 -- Уровень dB -5 to -7.5
    // - - - - - -  шаг 2.5 dB
    // 0xE     Уровень dB -35.0 to -37.5
    // 0xF     Уровень dB  Less than -37.5
    byte Liu::ReceiveLevel()
    {   	
        m_board.Write(L_INFO2, 0xff);
        return m_board.Read(L_INFO2) & 0xf;
    }
    
    //--------------------------------------------------------------------------------    
    
    byte Liu::UpdateBitsState()
    {
		m_board.isDevicePresent();    	
		LoadBitsState();
    	return m_stateLeds;
    }

    //--------------------------------------------------------------------------------    
    
    void Liu::LoadLOS()
    {

        m_board.Write(L_SR1, 0x08);
        
        if (m_board.Read(L_SR1) & 0x08) SetBit(m_stateLeds, L_STATE_LOS);
        						else	ClearBit(m_stateLeds, L_STATE_LOS);    	
    }
    
    //--------------------------------------------------------------------------------    
    
    void Liu::LoadAIS()
    {

   		m_board.Write(L_SR2, 0x4);

   		if (m_board.Read(L_SR2) & 0x4)	SetBit(m_stateLeds, L_STATE_AIS);
                        		else	ClearBit(m_stateLeds, L_STATE_AIS);
    }
     
    //--------------------------------------------------------------------------------    
    
    void Liu::LoadFas()
    {

        if(m_board.isT1Mode())
        {
            m_board.Write(L_SR2, 0x1);
   		    if (m_board.Read(L_SR2) & 0x1) SetBit(m_stateLeds, L_STATE_LOF);
            else ClearBit(m_stateLeds, L_STATE_LOF);
            return;
        }                
        
   	  	// fas,mfas,crc4
      	m_board.Write(L_INFO7, 0x6);
      	byte info7 = m_board.Read(L_INFO7) & 0x6;
      	if (info7 & 0x4)
      	{
			SetBit(m_stateLeds, L_STATE_LOF);      	
      	}
      	else
      	{
			ClearBit(m_stateLeds, L_STATE_LOF);      		
         	m_board.Write(L_INFO3, 0x2);
         	if(m_board.Read(L_INFO3) & 0x2) SetBit(m_stateLeds, L_STATE_LOF);
      	}

      	if ( (m_board.Read(L_E1RCR1) & 0x40))
		{
			SetBit(m_stateLeds, L_STATE_LOMF);
		}
		else
		{
			if (info7 & 0x6)
			{
				SetBit(m_stateLeds, L_STATE_LOMF);				
			}
			else
			{
				ClearBit(m_stateLeds, L_STATE_LOMF);
				m_board.Write(L_INFO3, 0x1);
				if(m_board.Read(L_INFO3)) SetBit(m_stateLeds, L_STATE_LOMF);
			}
		}    	
    }    

    //--------------------------------------------------------------------------------    
    
    void Liu::LoadRDI()
    {

		m_board.Write(L_SR3, 0x3);
		byte sr3 = m_board.Read(L_SR3);

		if (!TestBit(m_stateLeds, L_STATE_LOMF))
		{
			if (sr3 & 0x2) SetBit(m_stateLeds, L_STATE_RDI_MF);
					  else ClearBit(m_stateLeds, L_STATE_RDI_MF);
		}
		else
		{
			ClearBit(m_stateLeds, L_STATE_RDI_MF);		  
		}

		if (!TestBit(m_stateLeds, L_STATE_LOF))
		{
			if (sr3 & 0x1) SetBit(m_stateLeds, L_STATE_RDI);
					  else ClearBit(m_stateLeds, L_STATE_RDI);
		}
		else
		{
			ClearBit(m_stateLeds, L_STATE_RDI);
		}
    }
                   
    //--------------------------------------------------------------------------------    
    
    void Liu::LoadBert()
    {

		m_board.Write(L_SR9, 0xFF);
		byte sr9 = m_board.Read(L_SR9);		
		
		if (sr9 & 0x2) SetBit(m_stateLeds, L_STATE_LSS);
        		  else ClearBit(m_stateLeds, L_STATE_LSS);
        		          		 
      	if(!TestBit(m_board.Read(L_BIC), 0)) SetBit(m_stateLeds, L_STATE_LSS);

		if (((m_board.Read(L_BC1) >> 2) & 0x7) != 0x4)
		{
		 if (TestBit(sr9 ,3)) SetBit(m_stateLeds, L_STATE_LSS);
		 if (TestBit(sr9 ,2)) SetBit(m_stateLeds, L_STATE_LSS);         
		 if (!(m_board.Read(L_INFO2) & 0x80)) SetBit(m_stateLeds, L_STATE_LSS);
		}    	    	
    }    
    
	void Liu::LoadYellow()
	{
   		m_board.Write(L_SR2, 0x8);

   		if (m_board.Read(L_SR2) & 0x8)	m_yellowAlarm = true;
                        		else	m_yellowAlarm = false;	    
	}
    
    //--------------------------------------------------------------------------------
    //функция модифицирует m_stateLeds в соответствии с текущим состоянием линии
    
    void Liu::LoadBitsState()
	{
   	
		LoadLOS();
		LoadAIS();
		LoadFas();
		LoadRDI();
        LoadYellow();
		LoadBert();
				
}

    //--------------------------------------------------------------------
	//Функциия тестирования доступа и наличия чипа	
	
    bool Liu::isDS2155Present()
    {
        byte data;
        word i;

        // проверка наличия DS2155
        data = m_board.Read(L_IDR);
        if ((data & 0xf0 ) != 0xb0) return false;
        
        for (i =0; i <= 0xff; i++)
        {
            m_board.Write(L_IDR, (byte)(i));
            if ((data & 0xf0 ) != 0xb0) return false;
        }

        //проверка запись - чтение регистров
        for (i =0; i <=0xff ; i++)
        {
            m_board.Write(0xF0, (byte)(i));
            m_board.Write(0xF1, (byte)(i+1));
            m_board.Write(0xF2, (byte)(i+5));
            data = m_board.Read(0xF0);
            if (data != (byte)(i)) return false;
            data = m_board.Read(0xF1);
            if (data != (byte)(i+1)) return false;
            data = m_board.Read(0xF2);
            if (data != (byte)(i+5)) return false;
        }
        return true;
    }
    
	//------------------------------------------------		
			
	const PriStatus &Liu::PeekStatus()
	{
		m_board.isDevicePresent();		
		LoadBitsState();		
		m_status.rxLOS = m_stateLeds & ((byte)0x01 << L_STATE_LOS) ? true : false;
		m_status.rxAIS = m_stateLeds & ((byte)0x01 <<L_STATE_AIS) ? true : false;		
		m_status.rxLOF = m_stateLeds & ((byte)0x01 <<L_STATE_LOF) ? true : false;		
		m_status.rxLOMF = m_stateLeds & ((byte)0x01 <<L_STATE_LOMF) ? true : false;		
		m_status.rxLSS = m_stateLeds & ((byte)0x01 <<L_STATE_LSS) ? true : false;				
		m_status.rxYellowAlarm = m_yellowAlarm;
		m_status.rxSignalLevel = ReceiveLevel();
		return m_status;
	}    
    
    //---------------------------------------------------------------        
            
	void Liu::ChangeProfileType(const LiuProfile &src)
	{
	
//	    stop();
//	    m_profile.ChangeProfile(type);
//	    .....
//	    start();
	}
	
	//---------------------------------------------------------------------------	

};
