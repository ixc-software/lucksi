#include "stdafx.h"

#include <sys/exception.h>
#include <cdefBF537.h>
#include <ccblkfn.h>

#include "TdmRegMapping.h"
#include "Tdm.h"


namespace
{
    
    EX_INTERRUPT_HANDLER(Sport0RxIsr)
    {	
        BfTdm::Tdm::DMAInterrupt(0, BfTdm::Tdm::RxInterrupt);
    }   

    EX_INTERRUPT_HANDLER(Sport0TxIsr)   
    {	
        BfTdm::Tdm::DMAInterrupt(0, BfTdm::Tdm::TxInterrupt);
    }        
    
    

    EX_INTERRUPT_HANDLER(Sport1RxIsr)
    {	
        BfTdm::Tdm::DMAInterrupt(1, BfTdm::Tdm::RxInterrupt);
    }
    
    EX_INTERRUPT_HANDLER(Sport1TxIsr)   
    {	
        BfTdm::Tdm::DMAInterrupt(1, BfTdm::Tdm::TxInterrupt);
    }            

    
}    


namespace BfTdm
{
    
    enum
    {
        //количество SPORT портов
        Number_of_SPORTs  = 2,        
        //количество каналов в кадре            
        Number_of_Channels = 32,
    };        
    
    
   	//--------------------------------------------    
    
    TdmRegMap::TdmRegMap(word sportNum):
        m_sportNum(sportNum)
    {
        if (sportNum)
            FillBySport1Reg();
        else
            FillBySport0Reg();                
    }
    
   	//--------------------------------------------
	      
    void TdmRegMap::FillBySport0Reg()
    {
               
		*pPORT_MUX &= ~0x0007;
	    ssync();
        
       
        m_pDMARx_IRQ_Status = pDMA3_IRQ_STATUS;
        m_pDMATx_IRQ_Status = pDMA4_IRQ_STATUS;
        m_pSPORT_RClkDiv    = pSPORT0_RCLKDIV;
        m_pSPORT_RFSDiv     = pSPORT0_RFSDIV;
        
    	m_pSPORT_MTCS0      = pSPORT0_MTCS0;
    	m_pSPORT_MTCS1      = pSPORT0_MTCS1;
    	m_pSPORT_MTCS2      = pSPORT0_MTCS2;
    	m_pSPORT_MTCS3      = pSPORT0_MTCS3;
    	m_pSPORT_MRCS0      = pSPORT0_MRCS0;
    	m_pSPORT_MRCS1      = pSPORT0_MRCS1;
    	m_pSPORT_MRCS2      = pSPORT0_MRCS2;
    	m_pSPORT_MRCS3      = pSPORT0_MRCS3;
        	
    	m_pSPORT_RCR1       = pSPORT0_RCR1;
    	m_pSPORT_RCR2       = pSPORT0_RCR2;
    	m_pSPORT_TCR1       = pSPORT0_TCR1;
    	m_pSPORT_TCR2       = pSPORT0_TCR2;
    	m_pSPORT_MCMC1      = pSPORT0_MCMC1;
    	m_pSPORT_MCMC2      = pSPORT0_MCMC2;
        
        m_pDMARx_Config     = pDMA3_CONFIG;
        m_pDMARx_Next_Desc_Ptr = pDMA3_NEXT_DESC_PTR;
        m_pDMARx_X_Count    = pDMA3_X_COUNT;
        m_pDMARx_X_Modify   = pDMA3_X_MODIFY;
        m_pDMARx_Y_Count    = pDMA3_Y_COUNT;
        m_pDMARx_Y_Modify   = pDMA3_Y_MODIFY;
        
        m_pDMATx_Config     = pDMA4_CONFIG;
        m_pDMATx_Next_Desc_Ptr = pDMA4_NEXT_DESC_PTR;
        m_pDMATx_X_Modify   = pDMA4_X_MODIFY;
        m_pDMATx_X_Count    = pDMA4_X_COUNT;
        m_pDMATx_Y_Modify   = pDMA4_Y_MODIFY;
        m_pDMATx_Y_Count    = pDMA4_Y_COUNT;        
    }    
    
	//---------------------------------------------------------------------
	
    void TdmRegMap::FillBySport1Reg()
    {
        *pPORTG_FER |= 0xff00;
		*pPORT_MUX |= 0x0e00;
	    ssync();
        
        
        m_pDMARx_IRQ_Status = pDMA5_IRQ_STATUS;
        m_pDMATx_IRQ_Status = pDMA6_IRQ_STATUS;
        m_pSPORT_RClkDiv    = pSPORT1_RCLKDIV;
        m_pSPORT_RFSDiv     = pSPORT1_RFSDIV;
        
    	m_pSPORT_MTCS0      = pSPORT1_MTCS0;
    	m_pSPORT_MTCS1      = pSPORT1_MTCS1;
    	m_pSPORT_MTCS2      = pSPORT1_MTCS2;
    	m_pSPORT_MTCS3      = pSPORT1_MTCS3;
    	m_pSPORT_MRCS0      = pSPORT1_MRCS0;
    	m_pSPORT_MRCS1      = pSPORT1_MRCS1;
    	m_pSPORT_MRCS2      = pSPORT1_MRCS2;
    	m_pSPORT_MRCS3      = pSPORT1_MRCS3;
        	
    	m_pSPORT_RCR1       = pSPORT1_RCR1;
    	m_pSPORT_RCR2       = pSPORT1_RCR2;
    	m_pSPORT_TCR1       = pSPORT1_TCR1;
    	m_pSPORT_TCR2       = pSPORT1_TCR2;
    	m_pSPORT_MCMC1      = pSPORT1_MCMC1;
    	m_pSPORT_MCMC2      = pSPORT1_MCMC2;
        
        m_pDMARx_Config     = pDMA5_CONFIG;
        m_pDMARx_Next_Desc_Ptr = pDMA5_NEXT_DESC_PTR;
        m_pDMARx_X_Count    = pDMA5_X_COUNT;
        m_pDMARx_X_Modify   = pDMA5_X_MODIFY;
        m_pDMARx_Y_Count    = pDMA5_Y_COUNT;
        m_pDMARx_Y_Modify   = pDMA5_Y_MODIFY;
        
        m_pDMATx_Config     = pDMA6_CONFIG;
        m_pDMATx_Next_Desc_Ptr = pDMA6_NEXT_DESC_PTR;
        m_pDMATx_X_Modify   = pDMA6_X_MODIFY;
        m_pDMATx_X_Count    = pDMA6_X_COUNT;
        m_pDMATx_Y_Modify   = pDMA6_Y_MODIFY;
        m_pDMATx_Y_Count    = pDMA6_Y_COUNT;                
    }    

	//---------------------------------------------------------------------

    void TdmRegMap::InitDMAIntSport0()
    {
    	// configure interrupts
    	register_handler(ik_ivg8, Sport0RxIsr);		// assign ISR to interrupt vector
    	register_handler(ik_ivg9, Sport0TxIsr);		// assign ISR to interrupt vector
	
    	*pSIC_IAR0 &= 0xF00FFFFF;
    	*pSIC_IAR0 |= 0x02100000;
	
    			    	    	
    	*pSIC_IMASK |= IRQ_DMA3;	
    	*pSIC_IMASK |= IRQ_DMA4;			

        
    }    
	
	//---------------------------------------------------------------------

    void TdmRegMap::DisableDMAIntSport0()
    {
    			    	    	
    	*pSIC_IMASK &= ~IRQ_DMA3;	
    	*pSIC_IMASK &= ~IRQ_DMA4;			

        
    }        
    
	//---------------------------------------------------------------------
	
    void TdmRegMap::InitDMAIntSport1()
    {
        
    	// configure interrupts
    	register_handler(ik_ivg10, Sport1RxIsr);		// assign ISR to interrupt vector
    	register_handler(ik_ivg12, Sport1TxIsr);		// assign ISR to interrupt vector
	
    	*pSIC_IAR0 &= 0x0FFFFFFF;
    	*pSIC_IAR0 |= 0x30000000;

    	*pSIC_IAR1 &= 0xFFFFFFF0;
    	*pSIC_IAR1 |= 0x00000005;    	    	
	
    	*pSIC_IMASK |= IRQ_DMA5;	
    	*pSIC_IMASK |= IRQ_DMA6;			        
    	
    }    
    
	//---------------------------------------------------------------------
	
    void TdmRegMap::DisableDMAIntSport1()
    {
        
    	*pSIC_IMASK &= ~IRQ_DMA5;	
    	*pSIC_IMASK &= ~IRQ_DMA6;			        
    	
    }    
    
    
	//---------------------------------------------------------------------        
	
    void TdmRegMap::DisableDMAInt()
    {
        if (m_sportNum) DisableDMAIntSport1();
        else            DisableDMAIntSport0();
    }

	//---------------------------------------------------------------------        
	
    void TdmRegMap::InitDMAInt()
    {
        if (m_sportNum) InitDMAIntSport1();
        else            InitDMAIntSport0();
    }    
    
    //---------------------------------------------------------------------
    
    word TdmRegMap::GetSportNum() const
    {
        return m_sportNum;
    }
	
    //---------------------------------------------------------------------
    
    word TdmRegMap::GetSportsCount()
    {
        return Number_of_SPORTs;
    }
	    
    //---------------------------------------------------------------------
    word TdmRegMap::GetSportTimeSlotsCount()
    {
        return Number_of_Channels;
    }
	    
    //---------------------------------------------------------------------
    
	    
}; //namespace BfTdm

