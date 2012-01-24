#include "stdafx.h"
#include "BootControl.h"
#include "BootServer.h"
#include "BfBootCore/StartScript.h"
#include "Utils/ErrorsSubsystem.h"
#include "ExitAction.h"
#include "BoardSetup.h"
#include "BfBootCore/UserResponseCode.h"

namespace 
{
    using iLogW::EndRecord;
    using SBProtoExt::ISbpConnection;
    using namespace BfBootCore;

    const Platform::dword CCmdTimeout = 10000; // todo вынести в StartupParams

    // транслятор методы->команды протокола + отправщик
    class AdapterIClient : public BfBootCore::IBootServerToClient
    {
        Utils::SafeRef<ISbpConnection>& m_remoteIntf;
        iLogW::LogSession& m_log;
        iLogW::LogRecordTag m_tag;
    public:
        AdapterIClient(Utils::SafeRef<ISbpConnection>& remoteIntf, iLogW::LogSession& log) 
            : m_remoteIntf(remoteIntf),
            m_log(log),
            m_tag(m_log.RegisterRecordKindStr("SendResponse"))
        {}

    //IBootServerToClient impl:
    private:
        void RespLogin(const UserResponseInfo&, const std::string&, const std::string&, bool)
        {
            ESS_HALT("Supported on upper level");
        }
        void RespCmd(const BfBootCore::UserResponseInfo& code)
        {
            m_log << m_tag << "RespCmd with info " << code.AsString() << EndRecord;
            BfBootCore::ClientRespCmd::Send(m_remoteIntf->Proto(), code);
        }
        void RespGetParam(const std::vector<BfBootCore::ConfigLine>& userParams)
        {
            m_log << m_tag << "RespGetParam" << EndRecord;
            BfBootCore::ClientRespGetParam::Send(m_remoteIntf->Proto(), userParams);
        }
        void RespGetAppList(const BfBootCore::ImgDescrList& appList)
        {
            m_log << m_tag << "RespGetAppList" << EndRecord;
            BfBootCore::ClientRespGetAppList::Send(m_remoteIntf->Proto(), appList);
        }
        void RespNewImgAdded(const BfBootCore::ImgDescriptor& img)
        {
            m_log << m_tag << "RespNewImgAdded" << EndRecord;
            BfBootCore::ClientRespNewImgAdded::Send(m_remoteIntf->Proto(), img);
        }
        /*void DiscWithProtoError(const std::string& info)
        {
            ESS_HALT("Never call");
        }*/

        void RespGetScriptList(const ScriptList& scripts)
        {
            BfBootCore::ClientRespGetScriptList::Send(m_remoteIntf->Proto(), scripts);
        }

        void StateInfo(const BfBootCore::SrvStateInfo& info, Platform::int32 progress)
        {            
        	if (m_remoteIntf.IsEmpty()) return;
            BfBootCore::InfoState::Send(m_remoteIntf->Proto(), info, progress);            
        }
    };
} // namespace 



namespace BfBootSrv
{
    using iLogW::EndRecord;
    using namespace SBProto;    

    int CProcessProtoMsec = 20; // сколько и где должен быть задан?  

    // ------------------------------------------------------------------------------------

    // буффер образа загрузчика при обновлении
    class BootControl::BootImgBuffer
    {                        
        std::vector<byte> m_buffer;
        //std::vector<byte>::iterator m_pos;

        int m_pos;

    public:
        BootImgBuffer(dword size)
            : m_buffer(size),
            m_pos(0)
        {
            if (size == 0) ESS_THROW_T(BoardException, errSizeNull);            
        }

        dword getFreeSpace() const
        {
            return m_buffer.size() - m_pos;            
        }        

        void Write(const void* data, dword length) 
        {                       
            if (getFreeSpace() < length) ESS_THROW_T(BoardException, errNoEnoughtMemory);
            memcpy(&m_buffer.at(m_pos), data, length);
            m_pos += length;
        }

        bool Save(BoardSetup& cfg, Utils::SafeRef<IWriteProgress> progInd)
        {
            ESS_ASSERT(getFreeSpace() == 0);
            return cfg.WriteBootImg(m_buffer, progInd);            
        }

        dword getCRC()const
        {
            BfBootCore::CRC crc;
            crc.ProcessBlock(m_buffer);
            return crc.Release();
        }            
    };   

    // -------------------------------------------------------------------------------------

    BootControl::Sender::Sender( BootControl& owner)
        : m_pImpl(new AdapterIClient(owner.m_connected, *owner.m_logSession)),
        m_nextCmdTimeout(owner.m_nextCmdTimeout),
        m_owner(owner)
    {
    }

    BfBootCore::IBootServerToClient* BootControl::Sender::operator->() const
    {
        if (!m_owner.m_connected.IsEmpty())
        {
            m_nextCmdTimeout.Stop();
            m_nextCmdTimeout.Start();
        }
        
        return m_pImpl.get();
    }    


    // IBootCtrl impl:
    // ------------------------------------------------------------------------------------

    void BootControl::SetUserParam( const std::string& name, const std::string& value )
    {      
        m_cfg.SetUserParam(name, value);
        *m_logSession << "UserParam changed " << name << "=" << value << EndRecord;        
        m_pImplSend->RespCmd(BfBootCore::Ok);
    }

    // ------------------------------------------------------------------------------------

    void BootControl::SetFactory(Platform::dword hwNumber, Platform::dword hwType, const std::string& mac)    
    {   
        m_cfg.SetFactoryParams(hwNumber, hwType, mac);
        
        *m_logSession << "BordId updated: Num = " << hwNumber << "Type = " << EndRecord;                
        m_pImplSend->RespCmd(BfBootCore::Ok);
    }

    // ------------------------------------------------------------------------------------

    void BootControl::GetParamList() const
    {       
        std::vector<BfBootCore::ConfigLine> list;
        m_cfg.GetParamList(list);        
        m_pImplSend->RespGetParam(list);
    }

    // ------------------------------------------------------------------------------------

    void BootControl::GetAppList() const
    {                
        m_pImplSend->RespGetAppList( m_cfg.GetAppList() );
    }

    // ------------------------------------------------------------------------------------

    void BootControl::OpenWriteImg( dword size )
    {        
        m_cfg.OpenWriteImg(size);
        m_crcImg.Reset();
        *m_logSession << "Writing new image with Size = " << size << " was opened." << EndRecord;            
        m_pImplSend->RespCmd(BfBootCore::Ok);        
    }

    // ------------------------------------------------------------------------------------    

    void BootControl::WriteImgChunk(const void* pData, dword size )
    {
        m_cfg.WriteImgChunk(pData, size);
        m_crcImg.ProcessBlock(pData, size);
        *m_logSession << "Image chunk with size = " << size << " was writed." << EndRecord;            
        m_pImplSend->RespCmd(BfBootCore::Ok);                
    }

    // ------------------------------------------------------------------------------------

    void BootControl::CloseNewImg( const std::string& name, dword CRC )
    {                       
        if (m_crcImg.Release() != CRC)
        {
            m_cfg.AbortWriteApp();            
            ESS_THROW_T(BoardException, errCRC);            
        }
        const ImgDescriptor& descr = m_cfg.CloseNewImg(name, CRC);
        *m_logSession << "Writing new image complete. Image info: " << descr.ToString() << EndRecord;           
        m_pImplSend->RespNewImgAdded(descr);         
    }

    // ------------------------------------------------------------------------------------

    void BootControl::DeleteLastImg()
    {
        m_cfg.DeleteLastImg();
        *m_logSession << "Last image deleted." << EndRecord;  
        m_pImplSend->RespCmd(BfBootCore::Ok);
    }

    // ------------------------------------------------------------------------------------

    void BootControl::DeleteAllImg()
    {
        m_cfg.DeleteAllImg();
        *m_logSession << "All image deleted." << EndRecord;  
        m_pImplSend->RespCmd(BfBootCore::Ok);
    }

    // ------------------------------------------------------------------------------------

    void BootControl::SaveChanges()
    {
        m_cfg.SaveChanges();
        *m_logSession << "All changes was saved." << EndRecord;          
        m_pImplSend->RespCmd(BfBootCore::Ok);
    }

    // ------------------------------------------------------------------------------------

    void BootControl::OpenWriteBootImg( dword size )
    {       
        // todo use exception:        
        if (m_cfg.IsHaveUnsavedChanges()) ESS_THROW_T(BoardException, errHaveUnsavedChanges);
        if (m_bootImgBuffer) ESS_THROW_T(BoardException, errIsUploadState);
        if (m_cfg.getSpaceForNewBootImg() < size) ESS_THROW_T(BoardException, errNoEnoughtMemory);

        m_bootImgBuffer.reset(new BootImgBuffer(size));           
        *m_logSession << "Start buffering new boot image with declared Size = " << size << EndRecord;          
        m_pImplSend->RespCmd(BfBootCore::Ok);
    }

    // ------------------------------------------------------------------------------------

    void BootControl::WriteBooterChunk( const void* data, dword length )
    {        
        // todo use exception:
        if (!m_bootImgBuffer) ESS_THROW_T(BoardException, errNotOpened);
        if (m_cfg.IsHaveUnsavedChanges()) 
        {
            m_bootImgBuffer.reset();
            ESS_THROW_T(BoardException, errHaveUnsavedChanges);            
        }        

        m_bootImgBuffer->Write(data, length);
        *m_logSession << "Booter chunk was writed to buffer. Size = " << length << EndRecord; 
        m_pImplSend->RespCmd(BfBootCore::Ok);
    }

    // ------------------------------------------------------------------------------------

    void BootControl::CloseUpdateTransaction( dword releaseNumber )
    {
        m_cfg.CloseTransaction(releaseNumber);

        *m_logSession << "Transaction closed. New release number." << releaseNumber << EndRecord; 
        m_pImplSend->RespCmd(BfBootCore::Ok);
    }  

    // ------------------------------------------------------------------------------------

    void BootControl::CloseNewBootImg( dword CRC )
    {        
        if (!m_bootImgBuffer) ESS_THROW_T(BoardException, errNotOpened);
        
        if (m_cfg.IsHaveUnsavedChanges()) 
        {
            m_bootImgBuffer.reset();
            ESS_THROW_T(BoardException, errHaveUnsavedChanges);            
        }
        if (m_bootImgBuffer->getCRC() != CRC)
        {
            m_bootImgBuffer.reset();
            ESS_THROW_T(BoardException, UserResponseInfo(errCRC));
        }       

        *m_logSession << "Boot image received." << EndRecord; 
        m_pImplSend->RespCmd(BfBootCore::Ok);
        
        const bool saveComplete = m_bootImgBuffer->Save(m_cfg, this);        
        m_bootImgBuffer.reset();

        if (!saveComplete)
        {
            m_pImplSend->StateInfo(BfBootCore::infoSaveFailed);
            *m_logSession << "Save boot image failed." << EndRecord; 
            return;
        }

        *m_logSession << "Boot image updated." << EndRecord; 
        
        //SendInfo(BfBootCore::infoSaveComplete);        
        m_pImplSend->StateInfo(BfBootCore::infoSaveComplete); 
    }

    // ------------------------------------------------------------------------------------

    void BootControl::WritePercent( int prog )
    {
        m_pImplSend->StateInfo(BfBootCore::infoSaveProgress, prog);        
    }

    // ------------------------------------------------------------------------------------

    void BootControl::EraseDefaultCfg()
    {
        m_cfg.EraseDefaultCfg();
        *m_logSession << "Config in default region was erased! Next start will be in ServiceMode!" << EndRecord; 
        m_pImplSend->RespCmd(BfBootCore::Ok);
    }

    // ------------------------------------------------------------------------------------

    void BootControl::GetScriptList() const
    {
        m_pImplSend->RespGetScriptList( m_cfg.GetScriptList() );        
        m_pImplSend->RespCmd(BfBootCore::Ok);
    }


    // ------------------------------------------------------------------------------------

    //void BootControl::RunAditionalScript( const std::string& scriptName )
    //{
    //    if (!m_cfg.PrepareScript(scriptName)) ESS_THROW_T(BoardException, errCantRunScript);   
    //    *m_logSession << "Script validation complete. Set action RunScript after exit." << EndRecord;
    //    m_exitAction.SetRunScript(m_cfg.getPreparedScript());
    //    m_pImplSend->RespCmd(BfBootCore::Ok);           
    //    Platform::Thread::Sleep(50);        
    //    m_connected.Clear()->UnbindUser();
    //    m_exit.AsyncExit();       

    //    //if (scriptName == BfBootCore::NamedScript::NameOfBootScript()) ESS_THROW_T(BoardException, errCantRunScript); // не принципиально        
    //    //if ( !AsyncRunScript(scriptName) ) ESS_THROW_T(BoardException, errCantRunScript);   
    //    //
    //    //m_pImplSend->RespCmd(BfBootCore::Ok);           
    //}

    // ------------------------------------------------------------------------------------
    
    //todo: use BooterParam or IServer
    BootControl::BootControl(
        iCore::MsgThread& thread, 
        IBootCtrlOwner& exit, //?
        ExitAction& exitAction,
        BoardSetup& cfg,         
        Utils::SafeRef<iLogW::ILogSessionCreator> logCreator
        )
        : MsgObject(thread),
        m_cfg(cfg),        
        m_owner(exit),
        m_exitAction(exitAction),
        m_logSession( logCreator->CreateSession("BootControl", true) ),                  
        m_nextCmdTimeout(CCmdTimeout),
        m_pImplSend(*this),              
        m_tRun(this, &TMy::OnTimerRun),
        m_ctrlCmdConverter(*this)
    {
        //if (!m_cfg.IsServiceMode()) m_tRun.Start(cfg.getSrvConnectionTimeout());
        //if (!m_cfg.IsServiceMode()) m_tRun.Start(cfg.getSrvConnectionTimeout());
    }    

    // ------------------------------------------------------------------------------------
    
    void BootControl::OnTimerRun( iCore::MsgTimer* p)
    {
        ESS_ASSERT(p == &m_tRun);
        *m_logSession << "Timeout waiting connect. Try load by script." << EndRecord;                      

        if (!m_cfg.PrepareScript(BfBootCore::NamedScript::NameOfBootScript()))
        {
            *m_logSession << "Can`t run start-script. Server wait connection to fix error." << EndRecord;            
            return;
        }    

        *m_logSession << "Script validation complete." << EndRecord;
        AsyncRunScript();
    }    
    

    // ------------------------------------------------------------------------------------

    void BootControl::Connect( Utils::SafeRef<ISbpConnection> src )
    {
        ESS_ASSERT(m_connected != src && "This src allready connected");
        ESS_ASSERT(m_connected.IsEmpty() && "Connect without disconnect");
                
        m_connected = src;
        m_tRun.Stop();
        m_nextCmdTimeout.Start();
        *m_logSession << "Connected Client " << src->Name() 
            << "in " << ((m_cfg.IsServiceMode()) ? "ServiceMode" : "UserMode") 
            << EndRecord;
    }

    // ------------------------------------------------------------------------------------

    void BootControl::Disconnect( Utils::SafeRef<ISbpConnection> src, bool withSave)
    {
        ESS_ASSERT(m_connected == src);        

        //if (withSave) m_pImplSend->RespCmd(BfBootCore::Ok); // --?

        *m_logSession << "Client disconnected." << EndRecord;
        Clear();
        m_cfg.AbortWriteApp();

        if (m_cfg.IsHaveUnsavedChanges() && withSave) m_cfg.SaveChanges();
        if (m_cfg.HavePreparedScript()) return AsyncRunScript();
        if (m_cfg.IsHaveSavedCanges() || m_cfg.IsBooterUpdated()) return AsyncReload();        
        
        // было подключение, но ничего не изменяли
        if (!m_cfg.IsServiceMode() && E1App::Stack::IsInited()  && E1App::Stack::Instance().Started()) m_tRun.Start(m_cfg.getSrvConnectionTimeout());               
    }

    void BootControl::Clear()
    {
        if (m_bootImgBuffer) m_bootImgBuffer.reset();
        m_nextCmdTimeout.Stop();
        m_connected.Clear();        
    }

    // ------------------------------------------------------------------------------------
   
    BootControl::~BootControl()
    {

    }

    // ------------------------------------------------------------------------------------

    void BootControl::AsyncReload()
    {
        *m_logSession << "Board will be reload." << EndRecord;
        m_exitAction.SetReload();
        m_owner.AsyncExit();        
    }

    // ------------------------------------------------------------------------------------

    void BootControl::AsyncRunScript()
    {
        *m_logSession << "Set action RunScript after exit." << EndRecord;
        m_exitAction.SetRunScript(m_cfg.getPreparedScript());
        m_owner.AsyncExit(); 
    }

    // точка приема Sbp - комманд от клиента. Возвращает false если пакет нельзя обработать как команду
    bool BootControl::ProcessData( SBProto::SbpRecvPack& data ) 
    {
        try
        {            
            m_nextCmdTimeout.Stop();
            bool isProtoCmd = m_ctrlCmdConverter.Process(data);
            if (!isProtoCmd) m_nextCmdTimeout.Start();
            return isProtoCmd;
        }
        catch(BoardException& e) 
        {
            m_pImplSend->RespCmd(e.getProperty().AsCode());     
            return true;
        }        
        

        ESS_HALT("Never here");
        return false;
    }

    // -------------------------------------------------------------------------------------

    void BootControl::NetworkActivated()
    {
        if (!m_cfg.IsServiceMode() && m_connected.IsEmpty()) m_tRun.Start(m_cfg.getSrvConnectionTimeout());
    }
    

} // namespace BfBootSrv

