#include "ds2155_reg.h"
#include "Liu.h"
#include "BitUtils.h"
#include "Ds2155access.h"
#include "UartSimple.h"
#include <stdio.h>

    enum
    {
        DS2155_PCM30 = 0,
        DS2155_PCM31 = 1,
        DS2155_NO_STRUCT = 2,
    };

    enum
    {
        DS2155_AMI = 0,
        DS2155_HDB3 =1,
    };
    enum
    {
        DS2155_CRC_OFF,
        DS2155_CRC_RX,
        DS2155_CRC_TX,
        DS2155_CRC_RX_TX,
    };

    enum
    {
        DEF_FAS = 0x1B,
        DEF_NFAS= 0x40,
        DEF_MFAS= 0x0B
    };

/*
    enum
    {
        L_STATE_RDI_MF = 0,
        L_STATE_LSS,
        L_STATE_RDI,
        L_STATE_SERR,
        L_SYNC_LOSS,
        L_YELLOW_ALARM, 
        L_RED_ALARM,    // Loss of Sygnal
        L_BLUE_ALARM    // unframed all 1
    };    
*/    
    
    enum
    {
        L_STATE_RDI_MF = 0,
        L_STATE_LSS,
        L_STATE_RDI,
        L_STATE_SERR,
        L_STATE_LOF,
        L_STATE_LOMF,
        L_STATE_LOS,
        L_STATE_AIS
    };
    
    

    byte m_stateLeds;
	PriStatus m_status;

    
    //--------------------------------------------------------------------
	//Функциия тестирования доступа и наличия чипа	
	
    static bool isDS2155Present()
    {
        byte data;
        word i;

        // проверка наличия DS2155
        data = DSRead(L_IDR);
        if ((data & 0xf0 ) != 0xb0) return false;
        
        for (i =0; i <= 0xff; i++)
        {
            DSWrite(L_IDR, (byte)(i));
            if ((data & 0xf0 ) != 0xb0) return false;
        }

        //проверка запись - чтение регистров
        for (i =0; i <=0xff ; i++)
        {
            DSWrite(0xF0, (byte)(i));
            DSWrite(0xF1, (byte)(i+1));
            DSWrite(0xF2, (byte)(i+5));
            data = DSRead(0xF0);
            if (data != (byte)(i)) return false;
            data = DSRead(0xF1);
            if (data != (byte)(i+1)) return false;
            data = DSRead(0xF2);
            if (data != (byte)(i+5)) return false;
        }
        return true;
    }
    
    //--------------------------------------------------------------

    static void SaInsTransmit(byte sa8, byte sa7, byte sa6, byte sa5, byte sa4)
    {
		DSMaskClear(L_TSACR, 0x1F);
		DSMaskSet(L_TSACR, 0x1F);
        DSWrite(L_TSa8,sa8);
        DSWrite(L_TSa7,sa7);
        DSWrite(L_TSa6,sa6);
        DSWrite(L_TSa5,sa5);
        DSWrite(L_TSa5,sa4);
    }    
    
    //----------------------------------------------

    static void LongLine(bool mode)  // T1 Ready
    {
		DSMaskClear(L_LIC1, 0x10);        
        if (mode) DSMaskSet(L_LIC1, 0x10);			
    }   

    //---------------------------------------------------------------

    static void SignalNoStruct(bool mode)
    {
        if(mode)
        {
			DSMaskClear(L_E1TCR1, 0x80);
			DSMaskSet(L_E1TCR1, 0x80);
            DSWrite(L_E1TCR2,0);
            DSWrite(L_TSACR,0);
        }
        else
        {
			DSMaskClear(L_E1TCR1, 0x80);
			DSMaskSet(L_E1TCR1, 0x00);
            
        }
    }    
        
    //--------------------------------------------------------------    
    //функция установки структуры кадра (PCM30, PCM31, UnStruct)
    
    static bool Cycle(byte mode) 
    {
        if ( mode == DS2155_PCM30)
        {
            DSWrite( L_PCPR, 0x0);
			DSMaskSet(L_E1TCR1, 0x40);
            SignalNoStruct(false);
			DSMaskClear(L_E1RCR1, 0x40);
            return true;
        }

        if ( mode == DS2155_PCM31)
        {
            DSWrite(L_PCPR, 0x0);
            SignalNoStruct(false);
            DSMaskSet(L_E1RCR1, 0x40);
			DSMaskClear(L_E1TCR1, 0x40);            
            return true;
        }
 
        if (mode == DS2155_NO_STRUCT)
        {
                       
            DSWrite(L_PCPR,  0x0);
			DSMaskSet(L_E1RCR1, 0x40);
			DSMaskClear(L_E1TCR1, 0x40);
            SignalNoStruct(true);
            return true;
        }
        return false;
    }    
    
    // -------------------------------------------------------------
    
    static bool CRC4(byte mode)
    {
        if (mode)
        {
            //Включение генерации CRC4 на передачу
			DSMaskSet(L_E1TCR1, 0x01);            
            //Включение генерации CRC4 на передачу
			DSMaskSet(L_E1RCR1, 0x08);            
            //Генерация E бит (включается на передачу)
			DSMaskSet(L_E1TCR2, 0x04);                        
			
        }
        else
        {
			DSMaskClear(L_E1TCR1, 0x01);
			DSMaskClear(L_E1RCR1, 0x08);			
			DSMaskClear(L_E1TCR2, 0x04);						
        }
        return true;
    }

    //-----------------------------------------------------
    //функция сброса схемы синхронизации чипа
    static void ResetSync()                     //t1 ready
    {
        #ifdef T1_DEBUG
        	DSMaskClear(L_T1RCR1, 0x01);
    		DSMaskSet(L_T1RCR1, 0x01);                	
        	DSMaskClear(L_T1RCR1, 0x01);		            
        #else        
        	DSMaskClear(L_E1RCR1, 0x01);
    		DSMaskSet(L_E1RCR1, 0x01);                	
        	DSMaskClear(L_E1RCR1, 0x01);		
    	#endif
    }        
     

    
                    
   	//--------------------------------------------
	//Функциия инициализации чипа	    

#ifdef T1_ALARM
		
    bool LiuInitT1()
    {
    	
    	*pPORTFIO_DIR		|= 0x0010; //for SednaBoard
    	*pPORTFIO_SET 		= 0x0010;
    	*pPORTFIO_DIR		|= 0x4000;
    	*pPORTFIO_CLEAR		= 0x4000;
    	
    	
    	
        if (!isDS2155Present()) return false;
        
		*pPORTFIO_INEN		|= 0x8000;
        *pPORTF_FER	|= 0x8020;
		*pTIMER4_CONFIG		= 0x00AD;
		*pTIMER4_PERIOD		= 193;
		*pTIMER4_WIDTH		= 1;
		*pTIMER_ENABLE		|= 0x0010;        
		
	    
//        DSWrite(L_MSTRREG, 0x3); //e1
        DSWrite(L_MSTRREG, 0x1); //t1
        DSWrite(L_MSTRREG, 0x0);
        
//        DSWrite(L_IOCR1,   0x10);
        DSWrite(L_IOCR1,   0x00);
//        DSWrite(L_IOCR2,   0x3);
        DSWrite(L_IOCR2,   0x0);
        
//        DSWrite(L_T1CCR1,  0x02);        // delete zeros
//        T1FrameFill();
        
        
//        DSWrite(L_E1RCR1,  0x24);       
//        DSWrite(L_E1TCR1,  0x54);

//        DSWrite(L_T1RCR1,  0x24);
        DSWrite(L_T1RCR2,  0x20);        // B8ZS
//        DSWrite(L_T1TCR1,  0x54);
        DSWrite(L_T1TCR2,  0x80);        // B8ZS


        DSWrite(L_ESCR, 0x1);                

       //LIU
        DSWrite(L_LIC1, 0x01);
        
        
        DSWrite(L_LIC2, 0x18);        
//        DSWrite(L_LIC2, 0x90);
//        DSWrite(L_LIC3, 0x1);
        DSWrite(L_LIC3, 0x0);
        DSWrite(L_LIC4, 0x0f);
        
        
        DSWrite(L_TS1, 0x0b); //mfas

        for (byte i=0; i<15; i++)
        {
            DSWrite(L_TS2 + i, 0xFF);
        }

//        SaInsTransmit(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

        // выставление частоты синхронизации
        DSWrite(L_CCR1, 0x04);  //внутр
//        DSWrite(L_CCR1, 0x00);    //приним. 
  

        // 1 длинная линия; 0 короткая линия;
        LongLine(0);
        
/*      

        // 0-ami; 1-hdb3;
        if (1)
        {
            //hdb3
			  DSMaskSet(L_E1TCR1, 0x04);
			  DSMaskSet(L_E1RCR1, 0x20);			  
        }
        else
        {
            //ami
			  DSMaskClear(L_E1TCR1, 0x04);
			  DSMaskClear(L_E1RCR1, 0x20);			  
        }
        // 0-pcm 30 (CAS); 1-pcm 31 (CCS); 2 -no stuct
        Cycle(DS2155_PCM31);
        

        //0 -- no CRC-4, 1 -- CRC-4,
        CRC4(DS2155_CRC_OFF);
    
        //Установки FAS
        DSWrite(L_TAF, DEF_FAS);
        //Установка NFAS
        DSWrite(L_TNAF, DEF_NFAS);
        //Установка MFAS
        DSWrite(L_TS1,  DEF_MFAS);
        

//    	DSMaskClear(L_LIC2, 0x10); //AIS
*/                        
        // Сброс синхронизации
        ResetSync();

//    	DSMaskSet(L_T1TCR1, 0x02); //AIS

        return true;        
    }    
#else
    bool LiuInitT1()
    {
    	
    	*pPORTFIO_DIR		|= 0x0010; //for SednaBoard
    	*pPORTFIO_SET 		= 0x0010;
    	*pPORTFIO_DIR		|= 0x4000;
    	*pPORTFIO_CLEAR		= 0x4000;
    	
    	
    	
        if (!isDS2155Present()) return false;
        
		*pPORTFIO_INEN		|= 0x8000;
        *pPORTF_FER	|= 0x8020;
		*pTIMER4_CONFIG		= 0x00AD;
		*pTIMER4_PERIOD		= 193;
		*pTIMER4_WIDTH		= 1;
		*pTIMER_ENABLE		|= 0x0010;        
		
	    
//        DSWrite(L_MSTRREG, 0x3); //e1
        DSWrite(L_MSTRREG, 0x1); //t1
        DSWrite(L_MSTRREG, 0x0);
        
//        DSWrite(L_IOCR1,   0x10);
        DSWrite(L_IOCR1,   0x00);
        DSWrite(L_IOCR2,   0x3);  // rsysclk = 2048 
//         DSWrite(L_IOCR2,   0x0); // rsysclk = 1544 
                
        
//        DSWrite(L_E1RCR1,  0x24);
//        DSWrite(L_E1TCR1,  0x54);

//        DSWrite(L_T1RCR1,  0x24);
        DSWrite(L_T1RCR2,  0x20);        // B8ZS
//        DSWrite(L_T1TCR1,  0x54);
        DSWrite(L_T1TCR2,  0x80);        // B8ZS

//        DSWrite(L_T1CCR1,  0x02);
                        
        T1FrameFill();

        DSWrite(L_ESCR, 0x1);

       //LIU
        DSWrite(L_LIC1, 0x21);
        
//        DSWrite(L_LIC2, 0x10);           //2.048
        DSWrite(L_LIC2, 0x18);        // 1.544
//        DSWrite(L_LIC2, 0x90);
//        DSWrite(L_LIC3, 0x1);         // generate ALL 0xaa
        DSWrite(L_LIC3, 0x0);
        DSWrite(L_LIC4, 0x0f);
        
        
        DSWrite(L_TS1, 0x0b); //mfas

/*        
        for (byte i=0; i<15; i++)
        {
            DSWrite(L_TS2 + i, 0xFF);
        }
*/        

//        SaInsTransmit(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

        // выставление частоты синхронизации
//        DSWrite(L_CCR1, 0x04);  //внутр
//        DSWrite(L_CCR1, 0x00);    //приним. от TCLK 
        DSWrite(L_CCR1, 0x06);  //приним от RCLK, TCLK игнорируется
  
        
        DSWrite(L_CCR2, 0x07);    //приним.         

        // 1 длинная линия; 0 короткая линия;
        LongLine(0);


/*      
        
        // 0-ami; 1-hdb3;
        if (1)
        {
            //hdb3
			  DSMaskSet(L_E1TCR1, 0x04);
			  DSMaskSet(L_E1RCR1, 0x20);			  
        }
        else
        {
            //ami
			  DSMaskClear(L_E1TCR1, 0x04);
			  DSMaskClear(L_E1RCR1, 0x20);			  
        }
        // 0-pcm 30 (CAS); 1-pcm 31 (CCS); 2 -no stuct
        Cycle(DS2155_PCM31);
        

        //0 -- no CRC-4, 1 -- CRC-4,
        CRC4(DS2155_CRC_OFF);
    
        //Установки FAS
        DSWrite(L_TAF, DEF_FAS);
        //Установка NFAS
        DSWrite(L_TNAF, DEF_NFAS);
        //Установка MFAS
        DSWrite(L_TS1,  DEF_MFAS);
        

//    	DSMaskClear(L_LIC2, 0x10); //AIS

*/        
                
        // Сброс синхронизации
        ResetSync();

//    	DSMaskSet(L_T1TCR1, 0x02); //AIS

        return true;        
    }    
#endif    

   	//--------------------------------------------
	//Функциия инициализации чипа	    

    bool LiuInit()
    {
    	
    	*pPORTFIO_DIR		|= 0x0010; //for SednaBoard
    	*pPORTFIO_SET 		= 0x0010;
    	*pPORTFIO_DIR		|= 0x4000;
    	*pPORTFIO_CLEAR		= 0x4000;
    	
    	
    	
        if (!isDS2155Present()) return false;
        
		*pPORTFIO_INEN		|= 0x8000;
        *pPORTF_FER	|= 0x8020;
		*pTIMER4_CONFIG		= 0x00AD;
		*pTIMER4_PERIOD		= 256;
		*pTIMER4_WIDTH		= 1;
		*pTIMER_ENABLE		|= 0x0010;        			
    
        DSWrite(L_MSTRREG, 0x3);
        DSWrite(L_MSTRREG, 0x2);
        
//        DSWrite(L_IOCR1,   0x10);
        DSWrite(L_IOCR1,   0x0);
        DSWrite(L_IOCR2,   0x3);
        
        DSWrite(L_CCR1,    0x4);
        DSWrite(L_E1RCR1,  0x24);
        DSWrite(L_E1TCR1,  0x54);
        DSWrite(L_ESCR, 0x1);                

       //LIU
        DSWrite(L_LIC1, 0x21);
        DSWrite(L_LIC2, 0x90);
        DSWrite(L_LIC3, 0x0);
        DSWrite(L_LIC4, 0x0f);
        
        
        DSWrite(L_TS1, 0x0b); //mfas

        for (byte i=0; i<15; i++)
        {
            DSWrite(L_TS2 + i, 0xFF);
        }

        SaInsTransmit(0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

        // выставление частоты синхронизации
//        DSWrite(L_CCR1, 0x04);  //внутр
//        DSWrite(L_CCR1, 0x00);    //приним. 
        DSWrite(L_CCR1, 0x06);  //приним от RCLK, TCLK игнорируется
        DSWrite(L_CCR2, 0x07);  //включить генератор не BackPlan
        
        // 1 длинная линия; 0 короткая линия;
        LongLine(0);
        
        // 0-ami; 1-hdb3;
        if (1)
        {
            //hdb3
			  DSMaskSet(L_E1TCR1, 0x04);
			  DSMaskSet(L_E1RCR1, 0x20);			  
        }
        else
        {
            //ami
			  DSMaskClear(L_E1TCR1, 0x04);
			  DSMaskClear(L_E1RCR1, 0x20);			  
        }
        // 0-pcm 30 (CAS); 1-pcm 31 (CCS); 2 -no stuct
        Cycle(DS2155_PCM31);
        

        //0 -- no CRC-4, 1 -- CRC-4,
        CRC4(DS2155_CRC_RX_TX);
//        CRC4(DS2155_CRC_OFF);
    
        //Установки FAS
        DSWrite(L_TAF, DEF_FAS);
        //Установка NFAS
        DSWrite(L_TNAF, DEF_NFAS);
        //Установка MFAS
        DSWrite(L_TS1,  DEF_MFAS);
                        
        // Сброс синхронизации
        ResetSync();
        
        return true;        
    }        
                    
    //----------------------------------------------------------
    // Boзвращает уровень принимаемого сигнала
    // 0x0 -- Уровень dB  Greater than -2.5
    // 0x1 -- Уровень dB -2.5 to -5.0
    // 0x2 -- Уровень dB -5 to -7.5
    // - - - - - -  шаг 2.5 dB
    // 0xE     Уровень dB -35.0 to -37.5
    // 0xF     Уровень dB  Less than -37.5
    byte ReceiveLevel()
    {   	
        DSWrite(L_INFO2, 0xff);
        return DSRead(L_INFO2) & 0xf;
    }

    //--------------------------------------------------------------------------------
    //функция модифицирует m_stateLeds в соответствии с текущим состоянием линии

    //--------------------------------------------------------------------------------    
    
    static void LoadLOS()
    {

        DSWrite(L_SR1, 0x08);
        
        if (DSRead(L_SR1) & 0x08) SetBit(m_stateLeds, L_STATE_LOS);
        						else	ClearBit(m_stateLeds, L_STATE_LOS);    	
    }
    
    //--------------------------------------------------------------------------------    
    
    static void LoadAIS()
    {

   		DSWrite(L_SR2, 0x4);

   		if (DSRead(L_SR2) & 0x4)	SetBit(m_stateLeds, L_STATE_AIS);
                       		else	ClearBit(m_stateLeds, L_STATE_AIS);
    }
     
    //--------------------------------------------------------------------------------    
    
    static void LoadFas()
    {

        
        #ifdef T1_DEBUG
        
   		DSWrite(L_SR2, 0x1);

   		if (DSRead(L_SR2) & 0x1)	SetBit(m_stateLeds, L_STATE_LOF);
                       		else	ClearBit(m_stateLeds, L_STATE_LOF);
        
        
        #else
                
   	  	// fas,mfas,crc4
      	DSWrite(L_INFO7, 0x6);
      	byte info7 = DSRead(L_INFO7) & 0x6;
      	if (info7 & 0x4)
      	{
			SetBit(m_stateLeds, L_STATE_LOF);      	
      	}
      	else
      	{
			ClearBit(m_stateLeds, L_STATE_LOF);      		
         	DSWrite(L_INFO3, 0x2);
         	if(DSRead(L_INFO3) & 0x2) SetBit(m_stateLeds, L_STATE_LOF);
      	}

      	      	
      	if ( (DSRead(L_E1RCR1) & 0x40))
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
				DSWrite(L_INFO3, 0x1);
				if(DSRead(L_INFO3)) SetBit(m_stateLeds, L_STATE_LOMF);
			}
		}
    	#endif		
    }    

    //--------------------------------------------------------------------------------    
    
    static void LoadRDI()
    {

		DSWrite(L_SR3, 0x3);
		byte sr3 = DSRead(L_SR3);

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
    
    static void LoadBert()
    {

		DSWrite(L_SR9, 0xFF);
		byte sr9 = DSRead(L_SR9);		
		
		if (sr9 & 0x2) SetBit(m_stateLeds, L_STATE_LSS);
        		  else ClearBit(m_stateLeds, L_STATE_LSS);
        		          		 
      	if(!TestBit(DSRead(L_BIC), 0)) SetBit(m_stateLeds, L_STATE_LSS);

		if (((DSRead(L_BC1) >> 2) & 0x7) != 0x4)
		{
		 if (TestBit(sr9 ,3)) SetBit(m_stateLeds, L_STATE_LSS);
		 if (TestBit(sr9 ,2)) SetBit(m_stateLeds, L_STATE_LSS);         
		 if (!(DSRead(L_INFO2) & 0x80)) SetBit(m_stateLeds, L_STATE_LSS);
		}    	    	
    }               
        
    static void LoadBitsState()
	{   	
		LoadLOS();
		LoadAIS();
		LoadFas();
		LoadRDI();
		LoadBert();				
	}    
        
    //--------------------------------------------------------------------------------    
    
    static byte UpdateBitsState()
    {
		LoadBitsState();
    	return m_stateLeds;
    }

	//------------------------------------------------		
			
	void LoadStatus()
	{
		LoadBitsState();		
		m_status.rxLOS = m_stateLeds & ((byte)0x01 << L_STATE_LOS) ? true : false;
		m_status.rxAIS = m_stateLeds & ((byte)0x01 <<L_STATE_AIS) ? true : false;		
		m_status.rxLOF = m_stateLeds & ((byte)0x01 <<L_STATE_LOF) ? true : false;		
		m_status.rxLOMF = m_stateLeds & ((byte)0x01 <<L_STATE_LOMF) ? true : false;		
		m_status.rxLSS = m_stateLeds & ((byte)0x01 <<L_STATE_LSS) ? true : false;				
		m_status.rxSignalLevel = ReceiveLevel();
	}    

	void PriStatShow()
	{
		LoadStatus();
		sprintf(buff, "rxSignalLevel -%d dB\n", m_status.rxSignalLevel * 2,5);  		
		Send0Msg(buff);
		sprintf(buff, "rxLOS %d, rxAIS %d, rxLOF %d, rxLOMF %d, rxLSS %d \n", m_status.rxLOS, m_status.rxAIS, m_status.rxLOF, m_status.rxLOMF, m_status.rxLSS);
		Send0Msg(buff);
	}	
	
	void T1FrameFill()
	{
	    
	    DSWrite(L_IAAR, 1);        	    
	    DSWrite(L_PCICR, 0x81);
	    DSWrite(L_PCICR, 0xaa);	    
	    for(int i = 0; i< 22; i++) DSWrite(L_PCICR, 0x80);	    
	    DSWrite(L_TCICE1, 0xFE);
	    DSWrite(L_TCICE2, 0xFF);	    
	    DSWrite(L_TCICE3, 0xFF);
	    
    }	
	
