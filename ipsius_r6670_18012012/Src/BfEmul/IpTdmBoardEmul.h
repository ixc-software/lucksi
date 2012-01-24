#ifndef IPTDMBOARDEMUL_H
#define IPTDMBOARDEMUL_H

#include "Utils/IExitTaskObserver.h"
#include "E1App/E1AppConfig.h"

#include "BfBootSrv/LaunchBooter.h"
#include "BfBootCore/ServerTuneParametrs.h"
#include "FlashEmul.h"
#include "BfBootSrv/BoardSetup.h"
#include "BfBootSrv/ConfigSetup.h"
#include "BfBootSrv/LaunchBooter.h"
#include "E1App/E1AppRunner.h"


namespace BfEmul
{

    struct IpTdmBoardEmulProfile
    {   
        IpTdmBoardEmulProfile()
            : m_useBooter(false),
            m_softVer(-1),
            PAppExit(0)          
        {
            m_e1AppConfig.BroadcastSrcPort = 0; // bind port is any
            m_e1AppConfig.EchoMode = E1App::emNone;
        }

        bool m_useBooter;                
        int m_softVer;
        E1App::E1AppConfig m_e1AppConfig; 
        Utils::IExitTaskObserver *PAppExit;

        void SetBoardCfg(const BfBootCore::DefaultParam& cfg)        
        {
            m_pBoardCfg.reset( new BfBootCore::DefaultParam(cfg) );
        }
        bool ExistBoardCfg() const 
        {
            return m_pBoardCfg != 0;
        }
        const BfBootCore::DefaultParam& getBoardCfg()
        {
            return *m_pBoardCfg;
        }

    private:
        boost::scoped_ptr<BfBootCore::DefaultParam> m_pBoardCfg;

    };


    // Эмулятор платы IpTdm. Создается Utils::ThreadTaskRunner
    class IpTdmBoardEmul : boost::noncopyable,
        BfBootSrv::IReload
    {
    public:        

        IpTdmBoardEmul(boost::shared_ptr<IpTdmBoardEmulProfile> profile);

    // BfBootSrv::IReload
    private:
        void Reboot();
        void LoadToMain(BfBootSrv::BfStorage& storage, const BfBootCore::ImgDescriptor& img);
        void LoadToSpi(BfBootSrv::BfStorage& storage, const BfBootCore::ImgDescriptor& img);
        void LoadToSpi(Platform::dword size, const Platform::byte *data);

    private:
        
        void RunBooter();
        void RunE1App();
        void RunEcho();
        void Log(QString str)
        {
            //std::cout << "\n" << str << std::endl;
        }


        bool m_stateBootSrv; // or E1App
        bool m_stateEcho;

        // create if booter-stage used
        boost::scoped_ptr<BfBootSrv::BooterStartupParams> m_booterParams;         

        E1App::E1AppConfig m_e1AppConfig; 
        boost::scoped_ptr<BfRangeStorageDevEmul> m_flash;
        
        bool m_rebootFlag;
        
        int m_startBooterCounter;
        int m_startE1AppCounter;
    };
} // namespace BfEmul

#endif
