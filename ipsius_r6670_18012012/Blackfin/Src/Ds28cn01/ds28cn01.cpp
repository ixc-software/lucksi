#include "Ds28cn01\DS28CN01.h"

namespace
{
	Ds28cn01::DS28CN01 *SecChipObj = 0;


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
        CEEPROMWriteTimeOut = 50,        
	};
	
	enum
	{
        CUROMLen = 8,
        CPageNum = 4,
        CPageCapacity = 32
	};
	

}


namespace Ds28cn01
{
    	
    //-------------------------------------------------------------------

    DS28CN01::DS28CN01(BfDev::BfTwi &twi, word chipAddress):
    	m_twi(twi),
    	m_chipAddress(chipAddress)
    {
		ESS_ASSERT(!SecChipObj && "SecureChip already created");    	
		SecChipObj = this;
		
		if (!CheckSignature()) ESS_THROW(Absent);
        SetI2CMode();
    }
         
    //-------------------------------------------------------------------
    
    DS28CN01::~DS28CN01()
    {
		SecChipObj = 0;    	    	
    }
        
    //-------------------------------------------------------------------                
    
	MACBuffStruct DS28CN01::GetHash(const ChallendgeBuffStruct &challendge)
	{
        byte t_buff[ChallendgeBuffStruct::CSize + 2];
        t_buff[0] = CCmdAddress;
        t_buff[1] = CCmdMacCalcAnonim; 
        for(int i = 0; i< challendge.CSize; i++) t_buff[2+i] = challendge[i];
        if (m_twi.WriteData(m_chipAddress, t_buff, challendge.CSize + 2) != challendge.CSize + 2) ESS_THROW(Error);
        if (!m_twi.WaitAcknowledge(m_chipAddress, CEEPROMWriteTimeOut)) ESS_THROW(Error);
        t_buff[0] = CMacPageAddress;
        if (m_twi.WriteData(m_chipAddress, t_buff, 1) != 1) ESS_THROW(Error);
        MACBuffStruct result;                
        byte macbuff[MACBuffStruct::CSize];        
        if (m_twi.ReadData(m_chipAddress, macbuff, result.CSize) != result.CSize) ESS_THROW(Error);
        for(int i=0; i<result.CSize; i++) result[i] = macbuff[i];
        return result;
	}
	    
    //-------------------------------------------------------------------        
    
    void DS28CN01::SetI2CMode()
    {
        byte t_buff[2];
        t_buff[0] = CCommRegisterAddress;
        t_buff[1] = CCmdSetI2CMod; 
        if (m_twi.WriteData(m_chipAddress, t_buff, 2) != 2) ESS_THROW(Error);
    }
    
    //-------------------------------------------------------------------    		
	
	ddword DS28CN01::GetUIN()
	{
	    ddword result = GetUROM();
	    result <<= 8;	    
	    result >>= 16;
        return result;	    	    	    
	}    
	
    //-------------------------------------------------------------------    		
	
	ddword DS28CN01::GetUROM()
	{
        byte addr = CUINAddress;
        byte UROMbuff[CUROMLen];
	    ddword result = 0;        
        if (m_twi.WriteData(m_chipAddress, &addr, 1) != 1) ESS_THROW(Error);
        if (m_twi.ReadData(m_chipAddress, UROMbuff, CUROMLen) != CUROMLen) ESS_THROW(Error);
	    for(int i=0; i<CUROMLen; i++)
	    {
            result <<=8;            	        
            result += *(UROMbuff+ CUROMLen - 1 - i);
	    }
        return result;	                    
	}    	
    
    //-------------------------------------------------------------------    		
	
	void DS28CN01::WriteSecret(const SecretBuffStruct &secret)
    {
        byte t_buff[SecretBuffStruct::CSize+2];
        t_buff[0] = CCmdAddress;
        t_buff[1] = CCmdWriteSecret; 
        for(int i = 0; i< secret.CSize; i++) t_buff[2+i] = secret[i];
        if (m_twi.WriteData(m_chipAddress, t_buff, secret.CSize + 2) != secret.CSize + 2) ESS_THROW(Error);
        if(!m_twi.WaitAcknowledge(m_chipAddress, CEEPROMWriteTimeOut)) ESS_THROW(Error);                
    }	
	
    //-------------------------------------------------------------------    	
	
	byte DS28CN01::ReadPage(byte pageNum, byte *dataBuff)
	{
        ESS_ASSERT(pageNum < CPageNum && "DS28CN01 Error page");
        byte addr = pageNum * CPageCapacity;
        if (m_twi.WriteData(m_chipAddress, &addr, 1) != 1) ESS_THROW(Error);
        if (m_twi.ReadData(m_chipAddress, dataBuff, CPageCapacity) != CPageCapacity) ESS_THROW(Error);
        return CPageCapacity;
	}

    //-------------------------------------------------------------------            
    
    bool DS28CN01::CheckSignature()
    {
        byte crcVal = 0;        
        
	    ddword urom = GetUROM();	    
	    if ((byte)urom != CDS28CN01Signature) return false;        	    	    
	    
        for(int j = 0; j < CUROMLen; j++)
        {
            byte current_byte = urom;            
            for(int i = 0; i < 8; i++)
            {
                if(((current_byte ^ crcVal) & 0x01) != 0) crcVal = (((crcVal ^ 0x18) >> 1) | 0x80);
                else crcVal >>= 1;
                current_byte >>=1;                    
            }
            urom >>=8;
        }
        if (crcVal != 0) return false;
        return true;
    }

    //------------------------------------------	

	bool DS28CN01::isSecretClear()
	{
        byte InitHash[20] = {0xff, 0xd4, 0x92, 0x61, 0xa5, 0xb2, 0x59, 0xa9, 0x76, 0xe8, 0x50, 0xb3, 0xee, 0xae, 0xbe, 0xf1, 0x3e, 0xe8, 0xfc, 0x0};	    

        ChallendgeBuffStruct challendge;
        for(int i = 0; i< challendge.CSize; i++) challendge[i] = 0x00;
        
        MACBuffStruct hash = GetHash(challendge);
	    for(int i=0; i < hash.CSize; i++)
	    {        
	        if (InitHash[i] != hash[i]) return false;
	    }	    
        return true;        	    
	}
        			            
} //namespace DevIpTDM
