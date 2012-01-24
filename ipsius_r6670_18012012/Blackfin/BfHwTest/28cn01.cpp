
#include "Twi.h"
#include "28CN01.h"

namespace
{
	DevIpTDM::DS28CN01 *SecChipObj = 0;


	enum
	{
		CCmdAddress = 0xa9,
		CMacPageAddress = 0xb0,
        CCommRegisterAddress = 0xa8,
        CUINAddress = 0xa0,        		
		CCmdMacCalcAnonim = 0xe0,
		CCmdWriteSecret = 0xa0,		
        CCmdSetI2CMod = 0,
        CDS28CN01Signature = 0x70,                
	};

}


namespace DevIpTDM
{
    	
    //-------------------------------------------------------------------

    DS28CN01::DS28CN01(BfDev::BfTwi &twi, word chipAddress):
    	m_twi(twi),
    	m_chipAddress(chipAddress)
    {
//		ESS_ASSERT(!TWIObj && "TWI Port already created");    	
		SecChipObj = this;
				
//		ESS_THROW;    			
//		if(!SetI2CMode()) Send0Msg("Device not answer\n");				    				        
        SetI2CMode();
    }
         
    //-------------------------------------------------------------------
    
    DS28CN01::~DS28CN01()
    {
		SecChipObj = 0;    	    	
    }
        
    //-------------------------------------------------------------------                
    
	byte DS28CN01::GetHash(byte *hashBuff)
	{
        byte t_buff[9];
        t_buff[0] = CCmdAddress;
        t_buff[1] = CCmdMacCalcAnonim; 
        for(int i = 0; i< CChallengeLen; i++) t_buff[2+i] = *(hashBuff+i);
        if (m_twi.WriteData(m_chipAddress, t_buff, CChallengeLen + 2) != CChallengeLen + 2) return 0;
        if (!m_twi.WaitAcknowledge(m_chipAddress)) return 0;
        t_buff[0] = CMacPageAddress;
        if (m_twi.WriteData(m_chipAddress, t_buff, 1) != 1) return 0;
        if (m_twi.ReadData(m_chipAddress, hashBuff, CMacLen) != CMacLen) return 0;
        return CMacLen;
	}
	    
    //-------------------------------------------------------------------        
    
    bool DS28CN01::SetI2CMode()
    {
        byte t_buff[2];
        t_buff[0] = CCommRegisterAddress;
        t_buff[1] = CCmdSetI2CMod; 
        return (m_twi.WriteData(m_chipAddress, t_buff, 2) == 2);
    }
    
    //-------------------------------------------------------------------    		
	
	byte DS28CN01::GetUIN(byte *UINbuff)
	{
        byte t_buff[CUROMLen];	    
	    if (GetUROM(t_buff) != CUROMLen) return 0;
	    for(int i=0; i<CUINLen; i++) *(UINbuff + i) = *(t_buff+ CUINLen - i);        	    	    
        return CUINLen;	    
	}    
	
    //-------------------------------------------------------------------    		
	
	byte DS28CN01::GetUROM(byte *UINbuff)
	{
        byte addr = CUINAddress;
        if (m_twi.WriteData(m_chipAddress, &addr, 1) != 1) return 0;
        if (m_twi.ReadData(m_chipAddress, UINbuff, CUROMLen) != CUROMLen) return 0;
        return CUROMLen;	    
	}    	
    
    //-------------------------------------------------------------------    		
	
    bool DS28CN01::WriteSecret(byte *secretBuff)
    {
        byte t_buff[10];
        t_buff[0] = CCmdAddress;
        t_buff[1] = CCmdWriteSecret; 
        for(int i = 0; i< CSecretLen; i++) t_buff[2+i] = *secretBuff++;
        if (m_twi.WriteData(m_chipAddress, t_buff, CSecretLen + 2) != CSecretLen + 2) return false;
        return m_twi.WaitAcknowledge(m_chipAddress);                
    }	
	
    //-------------------------------------------------------------------    	
	
	byte DS28CN01::ReadPage(byte pageNum, byte *dataBuff)
	{
        if(pageNum >= CPageNum) return 0;
        byte addr = pageNum * CPageCapacity;
        if (m_twi.WriteData(m_chipAddress, &addr, 1) != 1) return 0;
        if (m_twi.ReadData(m_chipAddress, dataBuff, CPageCapacity) != CPageCapacity) return 0;
        return CPageCapacity;
	}

    //-------------------------------------------------------------------            
    
    bool DS28CN01::CheckSignature()
    {
        byte crcVal = 0;        
        
        byte UROMBuff[DevIpTDM::CUROMLen];      
	    if (GetUROM(UROMBuff) != CUROMLen) return false;        	    
	    if (UROMBuff[0] != CDS28CN01Signature) return false;        	    	    
	    
        for(int j = 0; j < CUROMLen; j++)
        {
            byte current_byte = UROMBuff[j];            
            for(int i = 0; i < 8; i++)
            {
                if(((current_byte ^ crcVal) & 0x01) != 0) crcVal = (((crcVal ^ 0x18) >> 1) | 0x80);
                else crcVal >>= 1;
                current_byte >>=1;                    
            }
        }
        if (crcVal != 0) return false;
        return true;
    }
		
	            
} //namespace DevIpTDM
