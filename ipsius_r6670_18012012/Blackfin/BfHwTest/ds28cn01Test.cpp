#include <stdio.h>


#include "Twi.h"
#include "28CN01.h"

#include "UartSimple.h"

namespace DevIpTdm
{
    byte InitHash[20] = {0xff, 0xd4, 0x92, 0x61, 0xa5, 0xb2, 0x59, 0xa9, 0x76, 0xe8, 0x50, 0xb3, 0xee, 0xae, 0xbe, 0xf1, 0x3e, 0xe8, 0xfc, 0x0};
    byte SecretHash[20] = {0x6a, 0x7, 0x42, 0x3b, 0x9d, 0x31, 0x5f, 0xb7, 0x76, 0x3d, 0xc4, 0x79, 0x89, 0xf, 0xc, 0x11, 0x5c, 0x92, 0x46, 0x52};        
    
//------------------------------------------	    
    
	bool DS28CN01Test()
	{	    
        BfDev::BfTwi TwiObj;
        DevIpTDM::DS28CN01 SecureChip(TwiObj, DevIpTDM::CSecureChipAddress);
        
	    return SecureChip.CheckSignature();
	}
	
//------------------------------------------	

	bool isDS28CN01Clear()
	{
        BfDev::BfTwi TwiObj;
        DevIpTDM::DS28CN01 SecureChip(TwiObj, DevIpTDM::CSecureChipAddress);
	            
        for(int i = 0; i< DevIpTDM::CChallengeLen; i++) buff[i]  = 0x00;
        byte len = SecureChip.GetHash((byte *)buff);
        if (len != DevIpTDM::CMacLen)
        {
            Send0Msg("GetHash: Invalid Chip\n");
            return false;
        }
	    for(int i=0; i<DevIpTDM::CMacLen; i++)
	    {        
	        if (InitHash[i] != (byte)buff[i]) return false;
	    }	    
        return true;        	    
	}
	
//------------------------------------------		

	void DS28CN01ShowInfo()
	{
	    {
            BfDev::BfTwi TwiObj;
            DevIpTDM::DS28CN01 SecureChip(TwiObj, DevIpTDM::CSecureChipAddress);
        
    	    if (SecureChip.GetUIN((byte *)&buff[150]) != DevIpTDM::CUINLen) Send0Msg("GetUIN: Invalid Chip\n");
            Send0Msg("UIN :");				    
    	    for(int i=0; i<DevIpTDM::CUINLen; i++)
    	    {
    		    sprintf(buff, " 0x%x", (unsigned char)buff[150 + i]);
                Send0Msg(buff);				    				        
    	    }	    
            Send0Msg("\n");	    
            
/*                        
            Send0Msg("Writing a secret\n");	    
            for(int i = 0; i< DevIpTDM::CSecretLen; i++) buff[150 + i]  = 0x00;
            if(!SecureChip.WriteSecret((byte *)&buff[150])) Send0Msg("ERROR Writing a secret\n");	    
            else Send0Msg("Secret OK\n");	                
*/            
            
    /*	    	    	    				            
            for(int i = 0; i< DevIpTDM::CChallengeLen; i++) buff[150 + i]  = 0x00;
            SecureChip.GetHash((byte *)&buff[150]);
            Send0Msg("HASH with 0x00 Challenge:");				    
    	    for(int i=0; i<DevIpTDM::CMacLen; i++)
    	    {
    		    sprintf(buff, " 0x%x", (unsigned char)buff[150 + i]);
                Send0Msg(buff);				    				        
    	    }
            Send0Msg("\n");
    */
        } 
        if (isDS28CN01Clear()) Send0Msg("SecureChip is clear\n");
        else Send0Msg("SecureChip is not clear\n");                    
	}
	
    //------------------------------------------		

	void DS28CN01LoadSecret()
	{
	    {
            BfDev::BfTwi TwiObj;
            DevIpTDM::DS28CN01 SecureChip(TwiObj, DevIpTDM::CSecureChipAddress);
                    

            byte secret[8] = {0xC6, 0xe5, 0xea, 0xe0, 0xd9, 0xd9, 0xd9, 0xd9};
                                    
            Send0Msg("Writing a secret\n");	    
//            for(int i = 0; i< DevIpTDM::CSecretLen; i++) buff[150 + i]  = 0x00;
            
            if(!SecureChip.WriteSecret(secret)) Send0Msg("ERROR Writing a secret\n");	    
            else Send0Msg("Secret OK\n");	                
            

            for(int i = 0; i< DevIpTDM::CChallengeLen; i++) buff[150 + i]  = 0x00;
            SecureChip.GetHash((byte *)&buff[150]);
            Send0Msg("HASH with 0x00 Challenge:");				    
    	    for(int i=0; i<DevIpTDM::CMacLen; i++)
    	    {
    		    sprintf(buff, " 0x%x", (unsigned char)buff[150 + i]);
                Send0Msg(buff);				    				        
    	    }
            Send0Msg("\n");

        } 
        if (isDS28CN01Clear()) Send0Msg("SecureChip is clear\n");
        else Send0Msg("SecureChip is not clear\n");                    
	}

//------------------------------------------	

	bool isSecretLoaded()
	{
        BfDev::BfTwi TwiObj;
        DevIpTDM::DS28CN01 SecureChip(TwiObj, DevIpTDM::CSecureChipAddress);
	            
        for(int i = 0; i< DevIpTDM::CChallengeLen; i++) buff[i]  = 0x00;
        byte len = SecureChip.GetHash((byte *)buff);
        if (len != DevIpTDM::CMacLen)
        {
            Send0Msg("GetHash: Invalid Chip\n");
            return false;
        }
	    for(int i=0; i<DevIpTDM::CMacLen; i++)
	    {        
	        if (SecretHash[i] != (byte)buff[i]) return false;
	    }	    
        return true;        	    
	}
		
		
};
