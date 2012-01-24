#include "stdafx.h"
#include "ApplicationLoader.h"
#include "BfStorage.h"
#include "BfDev/BfSpiLoader.h"

#include "DevIpTdm/BootFromSpi.h"
#include "DevIpTdm/DevIpTdmSpiMng.h"

#include "E1App/MngLwip.h"
#include "BfDev/BfWatchdog.h"


/*
    Загрузка приложений по Spi.
*/

namespace 
{
    const int CBlockSize = 512;
    const bool CTraceCout = true;
} // namespace  

namespace BfBootSrv	
{    
	
	void ApplicationLoader::Reboot() 
    {			
	    BfDev::BfWatchdog::SwReset();
	}
	
	ApplicationLoader::ApplicationLoader(const BfDev::SpiBusPoint &spiPt) :
        m_spiPt(new BfDev::SpiBusPoint(spiPt))
	{}    

    ApplicationLoader::ApplicationLoader()
    {}
	
    void ApplicationLoader::LoadToMain(BfBootSrv::BfStorage& storage, const BfBootCore::ImgDescriptor& img)
    {   
        ESS_ASSERT(m_spiPt);
        Platform::dword spiOffs = storage.getOffset(img);
                
        m_spiPt->Spi();
   		Platform::dword SpiBaud = BfDev::GetSpiBaud(m_spiPt->Spi().GetSpeedHz());  		   			
   			
   		if (CTraceCout) std::cout 
    		<< "Begin load MAIN."
    		<< "\nApplication info: " << img.ToString()
    		<< "\nSpiOffset = "     		
    		<< spiOffs
    		<< "\nSpiBaud = " << SpiBaud
    		<< std::endl;

    	if (E1App::Stack::IsInited())
		{
			E1App::Stack::Instance().Shutdown();		
			
			//iVDK::Sleep(500); // remove to stack?
		}
    		
        DevIpTdm::BootFromSpi(spiOffs, SpiBaud);   
             
        
        ESS_HALT("Never return");        
    }

	
    // --------------------------------------------------------


    void ApplicationLoader::LoadToSpi(BfBootSrv::BfStorage& storage, const BfBootCore::ImgDescriptor& img)
    {
        ESS_ASSERT(m_spiPt);

    	if (CTraceCout) std::cout 
    		<< "Begin load application through SPI. \nApplication info: " 
    		<< img.ToString()
    		<< std::endl;
        
    		    	
    	BfDev::BfSpiLoader loader(m_spiPt->Spi(),  *m_spiPt);

        storage.OpenForRead(img);

        std::vector<byte> buff;
        Platform::dword loadCount = 0;
        
        while (loadCount < img.Size)
        {
            int blockSize = CBlockSize;
            if (loadCount + blockSize > img.Size) blockSize =  img.Size - loadCount;
            storage.Read(buff, blockSize);
            if (!loader.LoadBlock(&buff.at(0), blockSize)) break;
            loadCount += blockSize;
        }

        storage.CloseRead();       
        
        if (CTraceCout) std::cout << "Application load finish." << std::endl;
    }
	
    // --------------------------------------------------------

	void ApplicationLoader::LoadToSpi(Platform::dword size, const Platform::byte *data)
	{    	
        ESS_ASSERT(m_spiPt);

    	BfDev::BfSpiLoader loader(m_spiPt->Spi(),  *m_spiPt);
    	
		loader.LoadBlock(data, size);
	}

} // namespace BfBootSrv
