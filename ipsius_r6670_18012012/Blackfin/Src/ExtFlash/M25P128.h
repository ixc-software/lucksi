#ifndef __M25P128__
#define __M25P128__

#include "Platform/Platform.h"
#include "flashMap.h"
#include "DevIpTdm/DevIpTdmSpiMng.h"
#include "BfDev/SpiBusMng.h"

namespace ExtFlash
{
    using namespace Platform;
    
	ESS_TYPEDEF(M25P128Absent);
	ESS_TYPEDEF(M25P128VerifyError);
	ESS_TYPEDEF(M25P128TimeOut);	
	   
    /*
        M25P128 class
        параметр syncMode = true переводит драйвер в синхронный режим, при этом метод ChipErase() будет работать около 250 сек,
        а SectorErase(num) около 4 сек. По умолчанию syncMode = false, т.е. драйвер работает в ассинхронном режиме и перед мспользованием методов:
	        void Read(dword offset, void *p, dword count);		
	        bool Write(dword offset, const void *p, dword count, bool verifyData);
	        void SectorErase(dword sector);
	        void ChipErase();					
        необходимо дождаться isReady(), иначе ESS_ASSERT
    */        
            
	class M25P128 : boost::noncopyable
	{				
		
	public:

        M25P128(const BfDev::SpiBusPoint &point, bool syncMode = false);
		~M25P128();
		const FlashMap &GetMap() const;
		const ChipSignature &GetSignature() const;

        // busy/ready state verify 
		bool isBusy();  
        bool isReady() { return !isBusy(); }

        // call both methods only in isReady() state 
		void Read(dword offset, void *p, dword count);		
		bool Write(dword offset, const void *p, dword count, bool verifyData);

        // for both methods
        // if syncMode == false then flash is in busy state after exit
		void SectorErase(dword sector);
		void ChipErase();					
					        
    private:
    
		bool Init();        
		
		void LoadSignature();
		void WriteEnable();		
		bool CheckM25P128Signature() const;

		void CheckBusy() const;
		byte GetStatus();
		void ReadyWait(dword msTime);
		bool VerifyPage(dword page, dword offset, const byte *pBuff, dword count);		
		bool WritePage(dword page, dword offset, const byte *pBuff, dword count, bool verifyData);
    
		enum
		{
			CMaxWritePageTimeMs = 10
		};
		
    private:
    	
        BfDev::SpiBusPoint m_point;
		BfDev::BfSpi	&m_spi;

		const FlashMap		m_flashMap;
		ChipSignature	m_signature;
		bool			m_busy;
		bool			m_syncMode;		

	};

} //namespace BfDev

#endif

