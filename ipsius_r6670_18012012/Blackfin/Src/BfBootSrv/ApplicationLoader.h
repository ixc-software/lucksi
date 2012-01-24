#ifndef APPLICATIONLOADER_H
#define APPLICATIONLOADER_H

#include "IReload.h"
#include "BfDev/SpiBusMng.h"

namespace BfBootSrv
{    
    // Имплементация IReload для реальной платы
    class ApplicationLoader : public IReload
    {
        boost::scoped_ptr<BfDev::SpiBusPoint> m_spiPt;
    	
    // IReload impl:
    public:
        void Reboot();
        void LoadToMain(BfBootSrv::BfStorage& storage, const BfBootCore::ImgDescriptor& img);        
        void LoadToSpi(BfBootSrv::BfStorage& storage, const BfBootCore::ImgDescriptor& img);
        void LoadToSpi(Platform::dword size, const Platform::byte *data);
        
    public:
    	ApplicationLoader(const BfDev::SpiBusPoint &spiPt);
        ApplicationLoader();
    };   
} // namespace BfBootSrv

#endif
