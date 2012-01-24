#ifndef BOOTCONTROL_H
#define BOOTCONTROL_H

#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"
#include "iLog/LogWrapper.h"
#include "BfBootCore/BootControlProto.h"
#include "BfBootCore/CRC.h"

#include "SafeBiProtoExt/ISbpConnection.h"

#include "IBootCtrlOwner.h"
#include "BfStorage.h"
#include "BfBootCore/BootControlProto.h"
#include "BfBootCore/NamedScript.h"
#include "TimeoutProcessor.h"
#include "FlashIOStream.h"


namespace BfBootSrv
{
    using boost::shared_ptr;
    using boost::scoped_ptr;    
    class BoardSetup;          
    class ExitAction;    

    
    class BootControl :
        boost::noncopyable,
        public iCore::MsgObject,
        public BfBootCore::IBootCtrl, // интерфейс команд от пользователя        
        public IWriteProgress 
    {
        typedef BootControl TMy;
        typedef shared_ptr<SBProto::SbpRecvPack> SbpSharedPack;                                        
        class BootImgBuffer;
        class Sender
        {
            scoped_ptr<BfBootCore::IBootServerToClient> m_pImpl;
            TimeoutProcessor& m_nextCmdTimeout;
            BootControl& m_owner;
        public:
            Sender(BootControl&);            
            BfBootCore::IBootServerToClient* operator->() const;
        };

        BoardSetup& m_cfg; 
        
        IBootCtrlOwner& m_owner;
        ExitAction& m_exitAction;
        
        mutable scoped_ptr<iLogW::LogSession> m_logSession;        
        TimeoutProcessor m_nextCmdTimeout;
                 
        Utils::SafeRef<SBProtoExt::ISbpConnection> m_connected; // точка подключения        
        Sender m_pImplSend;  // отправитель
        
        BfBootCore::CRC m_crcImg;        
        scoped_ptr<BootImgBuffer> m_bootImgBuffer; // существует в течение сессии обновления загрузчика              
        iCore::MsgTimer m_tRun;     // таймер ожидания подключения клиента  
        BfBootCore::BootCtrlConv m_ctrlCmdConverter; // конвертер команд протокола в вызовы методов IBootCtrl   

    // own methods:
    private:        
        void OnTimerRun(iCore::MsgTimer*);                                         
        void AsyncReload();                                
        void AsyncRunScript();        

    // IWriteProgress
    private:
        void WritePercent(int prog);

    // IBootCtrl impl:
    private:
        void SetUserParam(const std::string& name, const std::string& value);
        //void SetBoardId(dword HwNumber, dword HwType);
        void SetFactory(dword hwNumber, dword hwType, const std::string& mac);
        void GetParamList() const;
        void GetAppList() const;
        void OpenWriteImg(dword size);        
        void WriteImgChunk(const void* pData, dword size);
        void CloseNewImg(const std::string& name, dword CRC);
        void DeleteLastImg();
        void DeleteAllImg();
        //void AddScript(const BfBootCore::NamedScript& script);
        void CloseUpdateTransaction(dword releaseNumber);        
        void SaveChanges();
        void OpenWriteBootImg(dword size);
        void WriteBooterChunk(const void* pData, dword length);        
        void CloseNewBootImg(dword CRC);         
        void EraseDefaultCfg();        
        void GetScriptList() const;

        //void RunAditionalScript(const std::string& scriptName);

    public:               

        BootControl(
            iCore::MsgThread& thread, 
            IBootCtrlOwner& exit, //?
            ExitAction& exitAction,
            BoardSetup& cfg,             
            Utils::SafeRef<iLogW::ILogSessionCreator> logCreator);

        ~BootControl();

        void NetworkActivated();
        void Connect(Utils::SafeRef<SBProtoExt::ISbpConnection> src);        
        void Disconnect(Utils::SafeRef<SBProtoExt::ISbpConnection> src, bool withSave);        
        void Clear(); 
        
        // return true if protocol conversion complete
        bool ProcessData(SBProto::SbpRecvPack& data); // точка приема Sbp - комманд от клиента.;

        bool NextCmdTimeout()
        {
            return m_nextCmdTimeout.TimeoutExpired();
        }
    };
} // namespace BfBootSrv

#endif
