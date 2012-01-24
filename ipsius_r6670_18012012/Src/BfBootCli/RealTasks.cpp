#include "stdafx.h"

#include "BfBootCore/BootControlProto.h"
#include "BfBootCore/BooterVersion.h"

#include "RealTasks.h"
#include "BfBootCore/GeneralBooterConst.h"
#include "BfBootCore/CRC.h"
#include "BfBootCli/CFactorySettingsSupport.h"

#include "MiniFlasher/MfUtils.h"

namespace 
{
    
    enum TaskTimeout // msec
    {
        CSmolTimeout = 5 * 1000,
        CMediumTimeout = 30 * 1000,
        CHiTimeout = 60 * 1000,
    };

    int ChankSize()
    {
        const int CChankSize = 16 * 1024;
        BOOST_STATIC_ASSERT(CChankSize <= BfBootCore::CMaxChankSize);
        return CChankSize;
    }
} // namespace 

namespace BfBootCli
{            
    void TaskLogin::RunTask( BfBootCore::IClientToBooter& remoteServer )
    {
        m_pRemoteServer = &remoteServer;
        m_pRemoteServer->DoLogin(m_pwd, BfBootCore::CProtoVersion);
        m_waitResponseLogin = true;
    }

    // ------------------------------------------------------------------------------------

    QString TaskLogin::InfoTask() const
    {
        return QString("TaskLogin") + (m_waitResponseLogin ? " waiting response." : " inactive.");
    }

    // ------------------------------------------------------------------------------------

    void TaskLogin::RespLogin( const BfBootCore::UserResponseInfo& code, const std::string& additionalInfo, const std::string& booterVersion, bool userMode )
    {        
        if (!m_waitResponseLogin) return Halt("Unexpected ResponseLogin");
        m_waitResponseLogin = false;

        // failed
        if (code.AsCode() != BfBootCore::Ok) return Halt( "Rejected by server with code: " + code.AsString() + " Additional info: " + additionalInfo );

        if (userMode && m_mode == mod_ServiceAuto)
        {            
            ESS_ASSERT(m_pRemoteServer); // vs ignore
            m_waitCmdRsp = true;
            m_pRemoteServer->EraseDefaultCfg(); // wait RespCmd
            getOwner().Info(this, "Set service mode (erase factory settings)...");
            return;
        }
        
        if ( (userMode && m_mode == mod_Service) || (!userMode && m_mode == mod_User) ) return Halt("Booter connected in unexpected mode");


        //if (userMode != m_serverUserMode) return Halt("Booter connected in unexpected mode");
        //if (booterVersion != BfBootCore::CBooterVersion) return Halt("Unconformity BooterVersion"); // --- ?
        

        Complete();
    }

    void TaskLogin::RespCmd( const BfBootCore::UserResponseInfo& code )
    {
        if (!m_waitCmdRsp) return Halt("Unexpected RespCmd with code: " + code.AsString());
        m_waitCmdRsp = false;

        if (code.AsCode() != BfBootCore::Ok) return Halt("Rejected by server with code: " + code.AsString());
        getOwner().Info(this, "Set service mode complete.");

        Complete();
    }

    // ------------------------------------------------------------------------------------

    TaskLogin::TaskLogin( ITaskOwner& callback, const std::string& pwd, Mode mode ) 
        : TaskBase(callback, (mode == mod_ServiceAuto) ? CMediumTimeout : CSmolTimeout),            
        m_pwd(pwd),
        m_mode(mode),
        m_waitResponseLogin(false),
        m_waitCmdRsp(false),
        m_pRemoteServer(0)
    {
        //if (m_mode == mod_ServiceAuto && !BfBootCli::CFactorySettingsSupport) m_mode = mod_Service;
        if (m_mode == mod_ServiceAuto) ESS_ASSERT(BfBootCli::CFactorySettingsSupport);
    }    

    // ------------------------------------------------------------------------------------

    // ------------------------------------------------------------------------------------


    void TaskSetUserSettings::RunTask( BfBootCore::IClientToBooter& remoteServer )
    {
        remoteServer.SetUserParam(m_paramName, m_val);
    }

    // ------------------------------------------------------------------------------------

    QString TaskSetUserSettings::InfoTask() const
    {
        return QString("TaskSetUserSettings");
    }

    // ------------------------------------------------------------------------------------

    void TaskSetUserSettings::RespCmd( const BfBootCore::UserResponseInfo& code )
    {
        if (code.AsCode() != BfBootCore::Ok) Halt(code.AsString());
        Complete();
    }

    // ------------------------------------------------------------------------------------

    // ------------------------------------------------------------------------------------

    void TaskSetFactory::RunTask( BfBootCore::IClientToBooter& remoteServer )
    {
        remoteServer.SetFactory(m_hwNum, m_hwType, m_mac);
    }

    // ------------------------------------------------------------------------------------

    QString TaskSetFactory::InfoTask() const
    {
        return 
            QString("TaskSetFactory") + (IsActive() ? " waiting response." : " inactive.");
    }

    // ------------------------------------------------------------------------------------

    void TaskSetFactory::RespCmd( const BfBootCore::UserResponseInfo& code )
    {
        if (code.AsCode() != BfBootCore::Ok) return Halt(code.AsString());
        Complete();
    }

    // ------------------------------------------------------------------------------------

    TaskSetFactory::TaskSetFactory( ITaskOwner& callback, dword hwNum, dword hwType, std::string mac ) 
        : TaskBase(callback, CMediumTimeout),
        m_hwNum(hwNum),
        m_hwType(hwType),
        m_mac(mac)
    {
    }

    // ------------------------------------------------------------------------------------

    // ------------------------------------------------------------------------------------

    void TaskDeleteAllImg::RunTask( BfBootCore::IClientToBooter& remoteServer )
    {
        remoteServer.DeleteAllImg();
    }

    // ------------------------------------------------------------------------------------

    QString TaskDeleteAllImg::InfoTask() const
    {
        return QString("TaskDeleteAllImg") + (IsActive() ? " waiting response." : " inactive.");
    }

    // ------------------------------------------------------------------------------------

    void TaskDeleteAllImg::RespCmd( const BfBootCore::UserResponseInfo& resp )
    {
        if (resp.AsCode() == BfBootCore::Ok || resp.AsCode() == BfBootCore::errDelNothing) return Complete();
        Halt("Command DeleteAllImg was rejected with info" + resp.AsString());
    }

    // ------------------------------------------------------------------------------------

    // ------------------------------------------------------------------------------------

    void TaskCloseTransaction::RunTask( BfBootCore::IClientToBooter& remoteServer )
    {
        remoteServer.CloseUpdateTransaction(m_softwareReleaseNumber);
    }

    QString TaskCloseTransaction::InfoTask() const
    {
        return QString("TaskCloseTransaction");
    }

    void TaskCloseTransaction::RespCmd( const BfBootCore::UserResponseInfo& resp )
    {
        if (resp.AsCode() == BfBootCore::Ok) return Complete();
        Halt("Command was rejected with info" + resp.AsString());
    }

    TaskCloseTransaction::TaskCloseTransaction( ITaskOwner& callback, dword softwareReleaseNumber ) 
        : TaskBase(callback, CSmolTimeout),
        m_softwareReleaseNumber(softwareReleaseNumber)
    {
    }

    // ------------------------------------------------------------------------------------

    void TaskLogout::RunTask( BfBootCore::IClientToBooter& remoteServer )
    {
        remoteServer.DoLogout(m_withSave);
        // wait response?
    }

    // ------------------------------------------------------------------------------------

    QString TaskLogout::InfoTask() const
    {
        return QString("TaskLogout");
    }

    // ------------------------------------------------------------------------------------

    void TaskLogout::RespCmd( const BfBootCore::UserResponseInfo& resp )
    {
        if (resp.AsCode() == BfBootCore::Ok) return Complete();
        Halt("Command was rejected with info" + resp.AsString());
    }

    // ------------------------------------------------------------------------------------

    TaskLogout::TaskLogout( ITaskOwner& callback, bool withSave ) : TaskBase(callback),
        m_withSave(withSave)
    {
    }

    // ------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------

    //void TaskEraseDefaultCfg::RunTask( BfBootCore::IClientToBooter& remoteServer )
    //{
    //    remoteServer.EraseDefaultCfg();
    //}

    //// ------------------------------------------------------------------------------------

    //QString TaskEraseDefaultCfg::InfoTask() const
    //{
    //    return "TaskClearDefaultCfg";
    //}

    //// ------------------------------------------------------------------------------------

    //void TaskEraseDefaultCfg::RespCmd( const BfBootCore::UserResponseInfo& resp )
    //{
    //    if (resp.AsCode() != BfBootCore::Ok) return Halt("Command was rejected with info" + resp.AsString());
    //    Complete();
    //}

    //// ------------------------------------------------------------------------------------

    //TaskEraseDefaultCfg::TaskEraseDefaultCfg( ITaskOwner& callback )
    //    : TaskBase(callback, CHiTimeout)
    //{
    //}

    // ------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------

    void TaskSetTrace::RunTask( BfBootCore::IClientToBooter& remoteServer )
    {
        remoteServer.SetTrace(m_traceOn);
    }

    // ------------------------------------------------------------------------------------

    QString TaskSetTrace::InfoTask() const
    {
        return "TaskSetTrace";
    }

    // ------------------------------------------------------------------------------------

    void TaskSetTrace::RespCmd( const BfBootCore::UserResponseInfo& resp )
    {
        if (resp.AsCode() != BfBootCore::Ok) return Halt("Command was rejected with info" + resp.AsString());
        Complete();
    }

    // ------------------------------------------------------------------------------------

    TaskSetTrace::TaskSetTrace( ITaskOwner& callback, bool traceOn ) 
        : TaskBase(callback, CSmolTimeout), m_traceOn(traceOn)
    {
    }

    // ------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------

    TaskWrite::TaskWrite( ITaskOwner& callback, const BfBootCore::IHwFirmwareImage& img, WriteImgAs imgType ) 
        : TaskBase(callback),
        m_img(img),
        m_offset(0),            
        m_state(st_null),
        m_pRemoteServer(0),
        m_imgType(imgType),
        m_progTimeout(this, &TaskWrite::OnInfoTimeout),
        m_progInd(-1)
    {
        ESS_ASSERT(m_img.Data().size() != 0);
    }

    // ------------------------------------------------------------------------------------

    void TaskWrite::RunTask( BfBootCore::IClientToBooter& remoteServer )
    {
        ESS_ASSERT(m_state == st_null);               
        if (m_img.Data().size() == 0) return Complete(); // nothing to do    
        m_pRemoteServer = &remoteServer;

        if (m_imgType == AppImg) m_pRemoteServer->OpenWriteImg(m_img.Data().size());
        else /*BootImg*/         m_pRemoteServer->OpenWriteBootImg(m_img.Data().size());            

        m_state = st_waitOpen;
    }

    // ------------------------------------------------------------------------------------

    QString TaskWrite::InfoTask() const
    {                
        return QString("TaskWriteImage %1, State: %2, write progress %3%")
            .arg( m_img.Info().c_str() ).arg(ResolveState()).arg(Progress());
    }

    // ------------------------------------------------------------------------------------

    void TaskWrite::RespNewImgAdded( const BfBootCore::ImgDescriptor& descr ) /* */
    {
        if (m_state != st_waitAdded) return Halt("Unexpected response RespNewImgAdded");
        if (descr.Name != m_img.Info() || descr.Size != m_img.Data().size()) return Halt("Unexpected description of added file");

        Complete();
    }

    // ------------------------------------------------------------------------------------

    void TaskWrite::RespCmd( const BfBootCore::UserResponseInfo& resp ) /*  */
    {
        ESS_ASSERT(m_state != st_null);
        if (resp.AsCode() != BfBootCore::Ok) return Halt("Command was rejected with info" + resp.AsString());

        //stub! - skip one step
        //return;

        if (m_state == st_waitOpen) // ok opened
        {            
            m_timer.Reset();

            QString msg = QString("Send image started. ImgInfo: %1 size: %2 ")
                .arg(m_img.Info().c_str())
                .arg( MiniFlasherUtils::NumberFormat(m_img.Data().size()) );
            getOwner().Info(this, msg);            

            m_state = st_opened;                      
            WriteChunk();                
            return;
        }

        if (m_state == st_opened) // ok curr chunk writhed
        {
            int prog = Progress();
            if(prog != m_progInd)
                getOwner().Info(this, "Send image in progress.", prog);
            m_progInd = prog;

            if (!WriteChunk()) // конец передачи                                  
            {
                Platform::dword period = m_timer.Get();
                if (period == 0) period = 1;

                //Event report
                QString msg = 
                    QString("Send image complete. Time %1 sec. Speed %2 kBps.")
                    .arg(m_timer.Get() / 1000)
                    .arg(m_img.Data().size() / period);

                getOwner().Info(this, msg);
                
                BfBootCore::CRC crc;
                crc.ProcessBlock(m_img.Data().data(), m_img.Data().size());        

                if (m_imgType == AppImg) 
                {
                    m_pRemoteServer->CloseNewImg(m_img.Info(), crc.Release());
                    m_state = st_waitAdded;
                }
                else /*BootImg*/         
                {
                    m_pRemoteServer->CloseNewBootImg(crc.Release());
                    m_state = st_waitCRC;

                    
                }                    
            }      
            return;
        }

        if (m_state == st_waitCRC)
        {
            ESS_ASSERT(m_imgType == BootImg);

            m_timer.Reset();
            getOwner().Info(this, "Send complete. Server start verification & save by received image...");
            m_state = st_waitInfoSaveComplete;

            m_progTimeout.Start(CInfoTimeoutMsec);
            
//             const int CMinSaveSpeed = 30000;
//             SetTimeout(m_img.Data().size() / CMinSaveSpeed);

            return;
        }

        Halt("Unexpected response.");
    }

    // -------------------------------------------------------------------------------------

    void TaskWrite::StateInfo( const BfBootCore::SrvStateInfo& info, Platform::int32 progress )
    {   
        m_progTimeout.Stop();        

        if (info.AsCode() == BfBootCore::infoSaveComplete)
        {
            Platform::dword time = m_timer.Get();
            if (time == 0) time = 1; 

            QString msg = 
                QString("Server finish verification & save. Elapsed time %1 sec. Speed %2 kBps.")
                .arg(m_timer.Get() / 1000)
                .arg(m_img.Data().size() / time);

            getOwner().Info(this, msg);
            return Complete();
        }

        m_progTimeout.Start(CInfoTimeoutMsec);

        if (info.AsCode() == BfBootCore::infoSaveFailed) return Halt(info.AsString());        

        getOwner().Info(this, info.AsString().c_str(), progress);        
    }

    // ------------------------------------------------------------------------------------

    bool TaskWrite::WriteChunk()
    {       
        int rest = m_img.Data().size() - m_offset;
        if (rest == 0) return false;

        int size = ( rest < ChankSize() ) ?  rest : ChankSize(); 
        //int size = ( rest < 500 ) ?  rest : 500; 

        if (m_imgType == AppImg) m_pRemoteServer->WriteImgChunk(m_img.Data().data() + m_offset, size);
        else /*BootImg*/         m_pRemoteServer->WriteBooterChunk(m_img.Data().data() + m_offset, size);       

        m_offset += size;
        return true;
    }

    // ------------------------------------------------------------------------------------

    QString TaskWrite::ResolveState() const
    {
        // use meta ?
        switch (m_state)
        {
        case(st_null) : return "inactive";
        case(st_waitOpen): return "write opening";
        case(st_opened): return "write in progress";
        case(st_waitAdded): return "write closing";
        case(st_waitCRC): return "CRC verification";
        case(st_waitInfoSaveComplete) : return "server save received image";
        default: 
            ESS_HALT("InvalidState");
            return "";
        }
    }

    int TaskWrite::Progress() const
    {
        return m_offset * 100 / m_img.Data().size();
    }    

    void TaskWrite::OnInfoTimeout( iCore::MsgTimer* )
    {

    }
    // ------------------------------------------------------------------------------------

    TaskRunAddScript::TaskRunAddScript( ITaskOwner& callback, const std::string& scriptName, bool withSave ) 
        : TaskBase(callback, CSmolTimeout), m_scriptName(scriptName), m_withSave(withSave)
    {
    }

    void TaskRunAddScript::RespCmd( const BfBootCore::UserResponseInfo& resp )
    {
        if (resp.AsCode() != BfBootCore::Ok) return Halt("Command was rejected with info" + resp.AsString());
        Complete();
    }

    QString TaskRunAddScript::InfoTask() const
    {
        return "TaskRunAddScript";
    }

    void TaskRunAddScript::RunTask( BfBootCore::IClientToBooter& remoteServer )
    {
        remoteServer.RunAditionalScript(m_scriptName, m_withSave);
    }

    // ------------------------------------------------------------------------------------

    void TaskViewParams::RespGetParam( const std::vector<BfBootCore::ConfigLine>& userParams )
    {
        if (!m_waitList) return Halt("Unexpected RespGetParam");

        if (m_pResult) *m_pResult = userParams;

        for (int i = 0; i < userParams.size(); ++i)
        {
            QString val(userParams.at(i).Value().c_str());
            
            val.remove('\n');
            val.remove(BfBootCore::StringToTypeConverter::VectorBeginMarker().c_str());
            val.remove(BfBootCore::StringToTypeConverter::VectorEndMarker().c_str());
            val.replace(BfBootCore::StringToTypeConverter::VectorValueSep().c_str(), " ");

            QString info = QString("%1 = %2").arg(userParams.at(i).Name().c_str()).arg(val);                        

            getOwner().Info(this, info);
        }

        Complete();
    }

    void TaskViewParams::RunTask( BfBootCore::IClientToBooter& remoteServer )
    {
        m_waitList = true;
        remoteServer.GetParamList();
    }

    QString TaskViewParams::InfoTask() const
    {
        return "TaskViewParams";
    }

    TaskViewParams::TaskViewParams( ITaskOwner& callback, shared_ptr<std::vector<BfBootCore::ConfigLine> > pResult ) 
        : TaskBase(callback, CMediumTimeout), m_waitList(false),
        m_pResult(pResult)
    {
    }
} // namespace BfBootCli

