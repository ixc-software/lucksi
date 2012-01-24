#include "stdafx.h"

#include "ZL38065.h"


namespace DevIpTdm
{	
	
    // ------------------------------------------------------------------------------------
    
    ZL38065::ZL38065(IZl38065Transport &transport) :
    	m_transport(transport),  // can throw DeviceError
    	m_writeVerify(true)
    {
		if(!TryDetect(m_transport)) ESS_THROW(DeviceError);    	
    }
    
    
    // ------------------------------------------------------------------------------------        
    
    bool ZL38065::SetMode(int pair, bool enableExtended, bool enableLoCh, bool enableHiCh, std::string &extErrorInfo)
    {
    	
   	   	ESS_ASSERT(pair < CEchoCancellerCount/2 && "Requested EchoGroup is not exist");
    	
    	word baseAddr = pair * 0x40;
    	byte eca1, ecb1;
    	bool writeBoth = true;
    	bool verifyOk = true;
	
		if(enableExtended)
		{
			writeBoth = false;
			eca1 = 0x01;			
		}
		else
		{
			if(enableLoCh) eca1 = 0x00;
			else eca1 = 0x08;
			if(enableHiCh) ecb1 = 0x02;
			else ecb1 = 0x0A;
		}
										
		if(!WriteReg(baseAddr, eca1, m_writeVerify, extErrorInfo)) return false;
		if(!writeBoth) return true;		
		if(!WriteReg(baseAddr + 0x20, ecb1, m_writeVerify, extErrorInfo)) return false;		
    	return true;
    }
        
    // ------------------------------------------------------------------------------------        
    
    // return true if ZL38065 detected
    bool ZL38065::TryDetect(IZl38065Transport &transport)
    {    	
    	std::string temp;    	
    	dword pack;
		CodePackToBuff(pack, CRefAddr, 0xff, CRead);    	
		SendPack(pack, transport, temp);				
		PackPause();
    	dword result = SendPack(pack, transport, temp);		
    	if(isRequested(result, CRefAddr, CRead, temp))
    		if(GetData(result) == CRefVal) return true;
		return false;
    }
        
    //---------------------------------------------------------------------------	        
    
	//Pack Pause
	void ZL38065::PackPause()
	{
//		for(int i = 0; i<5000; ++i) asm("nop; nop; nop;"); //wait at least 5 ns in reset		
	}

    //---------------------------------------------------------------------------	    
    
	bool ZL38065::isRequested(dword pack, word addr, bool readOp, std::string &extErrorInfo)
	{
		std::ostringstream log;			
	
		if (pack & 0x1fff != addr)
		{
			
		    log<< "Echo_ *Address error; waiting 0x" << std::hex << (int)addr << "  incoming 0x" <<
		    std::hex << (int)(pack & 0x1fff) << std::endl;
		    extErrorInfo += log.str().c_str();	    
			return false;
		}
				
		bool bitBool = (pack & 0x2000);
		if (bitBool != readOp)
		{			
		    log<< "Echo_ Operation error; waiting " << BoolToOper(readOp) << "  incoming " <<
		    BoolToOper(bitBool) << std::endl;			
		    extErrorInfo += log.str().c_str();		    		    
			return false;		
		}	
		
		return true;
	}
	
    //---------------------------------------------------------------------------		
	
	byte ZL38065::GetData(dword pack)
	{
		return pack>>16;
	}
        
    //---------------------------------------------------------------------------			

	bool ZL38065::WriteReg(word addr, byte val, bool verify, std::string &extErrorInfo)
	{
	
    	dword wPack;		
    	dword rPack;		    	
    	dword inPack;		    	
    	std::string temp;    	
    	    	
		CodePackToBuff(wPack, addr, val, CWrite);
		SendPack(wPack, m_transport, temp);			
		if(!verify) return true;				
		CodePackToBuff(rPack, addr, 0xFF, CRead);
		PackPause();
		byte readAttempts = 0;
		byte writeAttempts = 1;		
					
		SendPack(rPack, m_transport, temp);		
		do
		{						
			readAttempts++;
			if(readAttempts > CMaxReadAttempts)
			{
				if(writeAttempts > CMaxWriteAttempts) break;
				else
				{
					writeAttempts++;
					readAttempts = 0;
					SendPack(wPack, m_transport, temp);
					SendPack(rPack, m_transport, temp);							
				}
				
			}
			inPack = SendPack(rPack, m_transport, temp);			
		}
		while(GetData(inPack) != val);
				
		
		if(GetData(inPack) != val)
		{
			std::ostringstream log;			
		    log<< "Echo_ Data error A=0x" << std::hex<< (int)addr  << " MB 0x" << std::hex << (int)val << "  IC 0x" <<
		    std::hex << (int)(GetData(inPack)) << std::endl;
		    extErrorInfo += log.str().c_str();		    		    
		} 
/*		
		else if(attempts > 1) 
		{
			std::ostringstream log;			
		    log<< "Echo_ Attempts=" << (int)attempts << "  A=0x" << std::hex<< (int)addr  << " MB 0x" << std::hex << (int)val << "  IC 0x" <<
		    std::hex << (int)(GetData(inPack)) << "  attempts =" << std::dec << (int)attempts << ";  " << std::endl;
		    extErrorInfo += log.str().c_str();		    		    					    
		}
*/		
		return isRequested(inPack, addr, CRead, extErrorInfo) && (GetData(inPack) == val);
	
	}
	
    //---------------------------------------------------------------------------	

	void ZL38065::CodePackToBuff(dword &pack, word addr, byte data, bool readOp)
	{
   	   	ESS_ASSERT(addr < 0x2000 && "Out of address space");		
		pack = data;
		pack <<= 8;		
		byte addrHi = (addr & 0xff00)>> 8;
		if(readOp) addrHi |= (0x01 << 5);			
		pack |= addrHi;
		pack <<= 8;										
		pack |= addr & 0xff;
	}    
	
    //---------------------------------------------------------------------------	

	dword ZL38065::SendPack(dword pack, IZl38065Transport &transport, std::string &extErrorInfo)
	{	
		dword result = 0;
		dword readMask;		
		byte timeOuts = 0;

		transport.ClearRxBuff();				
		
		for(byte i=0; i < CPackLen; ++i)
		{
			byte read;			
			if(!transport.WriteRead(pack, read))
			{
				read = 0xff;
				timeOuts++;
			}
			result |= read << (i*8);
			pack >>= 8;
		}		
		if(timeOuts)
		{
			std::ostringstream log;			
		    log<< "Echo_ Transport Timeouts = " << (int)timeOuts << std::endl;
		    extErrorInfo += log.str().c_str();		    		    			
		}
		return result;
	}    
	    
    // ------------------------------------------------------------------------------------            


} // namespace DevIpTdm
