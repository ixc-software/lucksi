#include "stdafx.h"

#include <ccblkfn.h>
#include <cdefBF537.h>

#include "M25P128.h"
#include "Utils/ErrorsSubsystem.h"



namespace ExtFlash
{
    
	M25P128 *M25P128Obj = 0;
	
    //-------------------------------------------------------------------

	M25P128::M25P128(const BfDev::SpiBusPoint &point, bool syncMode) :
        m_point(point),
    	m_spi( m_point.Spi() ),
		m_flashMap(64, 256 * 1024, 256),
		m_syncMode(syncMode)
    {
		ESS_ASSERT(M25P128Obj == 0 && "M25P128 already created");    	    	
		if (!Init()) ESS_THROW(M25P128Absent);		
		M25P128Obj = this;
		m_busy = isBusy();		
    }
    
    //-------------------------------------------------------------------            
    
	M25P128::~M25P128()
	{
		M25P128Obj = 0;
	}
    
    //-------------------------------------------------------------------        
    
	void M25P128::Read(dword offset, void *p, dword count)
	{
        byte *pBuff = static_cast<byte*>(p);
        
        ESS_ASSERT(m_flashMap.CorrectForRead(offset, count) && "Not Correct Space");
		CheckBusy();
		m_busy = true;
		{
            BfDev::SpiBusLocker busLock(m_point);
    		m_spi.Transfer(0x03);            //cmd
    		m_spi.Transfer(offset>>16);
    		m_spi.Transfer(offset>>8);
    		m_spi.Transfer(offset);
    		for (dword i = 0; i < count; i++)
    		{
    		  *pBuff = m_spi.Transfer(0xFF);      //read
    		  pBuff++;
    		}
		}//unlock SpiBus
		isBusy();
	}
	
    //-------------------------------------------------------------------    	
	
	bool M25P128::Write(dword offset, const void *p, dword count, bool verifyData)
	{	
        const byte *pBuff = static_cast<const byte*>(p);

        ESS_ASSERT(m_flashMap.CorrectForRead(offset, count) && "Not Correct Space");
		CheckBusy();
		dword inPageOffset = offset & 0xff;
		dword inPageCount;		
		dword page = offset>>8;				
		
		while (count)
		{
			if((inPageOffset + count) <= m_flashMap.PageSize) inPageCount = count;
			else inPageCount = m_flashMap.PageSize - inPageOffset;
			
//			ESS_ASSERT(WritePage(page, inPageOffset, pBuff, inPageCount, verifyData) && "M25P128:: Verify Error");
			if(!WritePage(page, inPageOffset, pBuff, inPageCount, verifyData)) ESS_THROW(M25P128VerifyError);
			
			count -= inPageCount;			
			pBuff += inPageCount;
			inPageOffset = 0;
			page++;
		}
		return true;
	}

    //-------------------------------------------------------------------    	
	
	bool M25P128::WritePage(dword page, dword offset, const byte *pBuff, dword count, bool verifyData)
	{
		m_flashMap.CheckPageSize(offset, count);
        ESS_ASSERT(m_flashMap.CorrectForRead(page * m_flashMap.PageSize, offset + count) && "Not Correct Space");		
		CheckBusy();
		m_busy = true;				
		
		const byte *tempBuff = pBuff;

		WriteEnable();
		{
            BfDev::SpiBusLocker busLock(m_point);
    		m_spi.Transfer(0x02);
    		m_spi.Transfer(page>>8);
    		m_spi.Transfer(page);
    		m_spi.Transfer(offset);
    		for (dword i = 0; i < count; i++)
    		{
    			m_spi.Transfer(*tempBuff);
    			tempBuff++;
    		}
		}//unlock SpiBus
		
		ReadyWait(CMaxWritePageTimeMs);
			
		if (verifyData) return VerifyPage(page, offset, pBuff, count);
		return true;

	}		

	//-------------------------------------------------------------------	
	
	bool M25P128::VerifyPage(dword page, dword offset, const byte *pBuff, dword count)
	{
		long int pageAddress = page * m_flashMap.PageSize;
		
		m_flashMap.CheckPageSize(offset, count);
        ESS_ASSERT(m_flashMap.CorrectForRead(pageAddress, offset + count) && "Not Correct Space");		
		CheckBusy();
		
		vector<byte> rdBuff;
		rdBuff.resize(count);
		
		Read(pageAddress + offset, &rdBuff[0], count);

		for(dword i=0; i<count; i++)
		{
			if(rdBuff[i] != *pBuff) return false;
			pBuff++;
		}	
		return true;  					
	}

	//-------------------------------------------------------------------

	void M25P128::ChipErase()
	{
        CheckBusy();
        	    
		m_busy = true;
		WriteEnable();		
		{
            BfDev::SpiBusLocker busLock(m_point);
    		m_spi.Transfer(0xc7);
        }//unlock SpiBus
		
        if (m_syncMode)
        {
            ReadyWait(250 * 1000);
        }		
	}
	
	//-------------------------------------------------------------------

	void M25P128::SectorErase(dword sector)
	{
		ESS_ASSERT(m_flashMap.CorrectSector(sector) && "Sector out of space");    			

        CheckBusy();
		
		m_busy = true;
		dword address = sector * m_flashMap.SectorSize;
		WriteEnable();		
		{
            BfDev::SpiBusLocker busLock(m_point);
    		m_spi.Transfer(0xd8);
    		m_spi.Transfer(address>>16);
    		m_spi.Transfer(address>>8);
    		m_spi.Transfer(address);		
		}//unlock SpiBus

        if (m_syncMode)
        {
            ReadyWait(10 * 1000);
        }
	}
			
	//-------------------------------------------------------------------

	bool M25P128::isBusy()
	{
		m_busy = GetStatus() & 0x01;
		return m_busy;
	}
		
	//-------------------------------------------------------------------

	void M25P128::ReadyWait(dword msTime)
	{
		int tryMs = 0;
		while(isBusy()) 
		{
			iVDK::Sleep(1);			
			tryMs++;
//			ESS_ASSERT(tryMs <= msTime && "M25P128:: Timeout");
			if(tryMs > msTime) 
			{
				ESS_THROW(M25P128TimeOut);
				break;
			}
		}
	}
		
    //-------------------------------------------------------------------    		
	
	byte M25P128::GetStatus() 
	{
        BfDev::SpiBusLocker busLock(m_point);
		m_spi.Transfer(0x05);
		byte result = m_spi.Transfer(0xFF);
		return result;
	}	

    //-------------------------------------------------------------------    
    
	void M25P128::CheckBusy() const
	{
		ESS_ASSERT( !m_busy  && "M25P128:: Busy");				
	}		
    
    //-------------------------------------------------------------------    
    
	const FlashMap &M25P128::GetMap() const
	{
		return m_flashMap;
	}
	
    //-------------------------------------------------------------------    
    
	const ChipSignature &M25P128::GetSignature() const
	{
		return m_signature;
	}	
    
    //-------------------------------------------------------------------	
	     
	void M25P128::WriteEnable()
	{
        BfDev::SpiBusLocker busLock(m_point);
		m_spi.Transfer(0x06);
	}
	
    //-------------------------------------------------------------------	
	
	void M25P128::LoadSignature()
	{

        BfDev::SpiBusLocker busLock(m_point);	    
		m_spi.Transfer(0x9f);
		m_signature.Manufacturer = m_spi.Transfer(0xFF);
		m_signature.MemType = m_spi.Transfer(0xFF);
		m_signature.MemCapacity = m_spi.Transfer(0xFF);
				
	}
	
    //-------------------------------------------------------------------
		
	bool M25P128::CheckM25P128Signature()	const
	{
		if (m_signature.Manufacturer != 0x20) return false;
		if (m_signature.MemType != 0x20) return false;		
		if (m_signature.MemCapacity != 0x18) return false;
		return true;
	}
	
    //-------------------------------------------------------------------

	bool M25P128::Init()
	{					
		WriteEnable();	
		LoadSignature();
		return CheckM25P128Signature();
	}
		
    
} //namespace BfDev
