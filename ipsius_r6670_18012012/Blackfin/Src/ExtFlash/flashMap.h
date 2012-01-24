#ifndef __FLASH_MAP__
#define __FLASH_MAP__

#include "Platform/PlatformTypes.h"

namespace ExtFlash
{
    using namespace Platform;
    using namespace std;    

    struct ChipSignature	// by JEDEC spec
    {
    	int	Manufacturer;
    	int	MemType;    	
    	int MemCapacity;    	

		string ToString(bool withLf = true) const
		{				
            string lf = withLf ? "\n" : "  ";

			ostringstream resStream;

			resStream << "manufacturer : " << ResolveManufacturer(Manufacturer) << lf;
			resStream << "memType : " << ResolveMemoryType(MemType) << lf;			
			resStream << "memCapacity : " << ResolveMemorySize(MemCapacity) << lf;			
	
			return resStream.str();
		}  
	
		private:		
		
		static const string ResolveManufacturer(int	manufacturer)
		{
			ostringstream resStream;
			
			if(manufacturer == 0x20) resStream << "ST";
			else resStream  << "0x" << hex << manufacturer;
			return resStream.str();						
		}
		
		static const string ResolveMemoryType(int memType)
		{
			ostringstream resStream;
			
			if(memType == 0x20) resStream << "Flash Memory";
			else resStream << "0x" << hex << memType;
			return resStream.str();						
		}
		
		static const string ResolveMemorySize(int memCapacity)
		{
			ostringstream resStream;
			
			if(memCapacity == 0x18) resStream << "128 Mbit";
			else resStream << "0x"  << hex << memCapacity;
			return resStream.str();						
		}		
		
    	    	
    };
    
	//----------------------------------------------------------      
   
    struct FlashMap
    {
    	dword	TotalBytes;
    	dword	Sectors;    	
    	dword	SectorSize;    	
    	dword	Pages;    	
    	dword	PageSize;    	    	
    	
		FlashMap(dword sectors, dword sectorSize, dword pageSize):
			TotalBytes(sectors * sectorSize),	
			Sectors(sectors),
			SectorSize(sectorSize),
    		Pages(sectors * sectorSize/pageSize),
    		PageSize(pageSize)
		{		
		}
				
        bool CorrectSector(int sector) const
        {
            return (sector >= 0) && (sector < Sectors);
        }

        bool CorrectForWrite(int offs, int size, bool verifySizeAlign) const
        {
            if (offs < 0)                   return false;
            if (offs % PageSize != 0)       return false;

            if (size <= 0)                  return false;
            if (verifySizeAlign)
            {
                if (size % PageSize != 0)     return false;
            }

            return true;
        }

        bool CorrectForRead(int offs, int size) const
        {
            if (offs < 0) return false;
            if (size <= 0) return false;

            if (offs + size > TotalBytes) return false;
            
            return true;
        }
    
    	bool CheckPageSize(int offset, int count) const
    	{
            if (offset < 0) return false;    	    
            if (count <= 0) return false;            
    		return offset + count <= PageSize;
    	}			            
        
		
	//----------------------------------------------------------        		
    
		string ToString(bool withLf = true) const
		{				
            string lf = withLf ? "\n" : "  ";

			ostringstream resStream;

			resStream << "capacity : "      << (long int)TotalBytes << lf;
			resStream << "sectors : "       << (long int)Sectors    << lf;			
			resStream << "sectorSize : "    << (long int)SectorSize << lf;			
			resStream << "pages : "         << (long int)Pages      << lf;			
			resStream << "pageSize : "      << (long int)PageSize   << lf;						
	
			return resStream.str();
		}  
		
	//----------------------------------------------------------        		
		
    };

} //namespace ExtFlash

#endif

