#ifndef REALTASKS_H
#define REALTASKS_H

#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"
#include "BfBootCore/IHwFirmware.h"

#include "RealTaskBase.h"
#include "ITaskOwner.h"

#include "BfBootCore/StringToTypeConverter.h"

/*
    »мплементации ITask - конкретные задани€.
    «адани€ создаютс€ только динамически, получа€ в конструктор ссылку на владельца, у которого они регистрируютс€.
    ѕор€док создани€ соответствует пор€дку регистрации и пор€дку выполнени€
*/


namespace BfBootCli
{        
    using Platform::word;
    using Platform::byte;
    using Platform::dword;    

    // ------------------------------------------------------------------------------------

    class TaskLogin 
        : public TaskBase
    {                        
    public:

        enum Mode
        {
            mod_User, // ожидаетс€ подключени в User режиме
            mod_Service, // ожидаетс€ подключени в Service режиме            
            mod_ServiceAuto // ожидаетс€ подключени в Service режиме, если нет - то сброс фабричных настроек
        };
        TaskLogin(
            ITaskOwner& callback,             
            const std::string& pwd,
            Mode mode = mod_User
            );
        

    // IProtoObserver impl:    
    private:
        void RunTask( BfBootCore::IClientToBooter& remoteServer );
        QString InfoTask() const;

    // IProtoObserver
    private:
        
        void RespLogin(const BfBootCore::UserResponseInfo& code, const std::string& additionalInfo, const std::string& booterVersion, bool userMode);        
        void RespCmd(const BfBootCore::UserResponseInfo& code);

        const std::string m_pwd;                        
        const Mode m_mode;
        bool m_waitResponseLogin; 
        bool m_waitCmdRsp;
        BfBootCore::IClientToBooter* m_pRemoteServer;
    
    };

    // ------------------------------------------------------------------------------------
        
    class TaskSetUserSettings
        : public TaskBase
    {        
         const std::string m_val;               
         const std::string m_paramName;
    
    // TaskBase override:
    private:    
        void RunTask( BfBootCore::IClientToBooter& remoteServer );
        QString InfoTask() const;

    private:
        void RespCmd(const BfBootCore::UserResponseInfo& code);
    public:
        template<class TParam>
        TaskSetUserSettings(
            ITaskOwner& callback,
            const TParam& val,
            const std::string& paramName
            )
            : TaskBase(callback, 10000),
            m_val( BfBootCore::StringToTypeConverter::toString(val) ),
            m_paramName(paramName)
        {}
    };

    // ------------------------------------------------------------------------------------

    class TaskSetFactory
        : public TaskBase
    {        
        const dword m_hwNum;               
        const dword m_hwType;
        const std::string m_mac;               

    // TaskBase override:
    private:
        void RunTask( BfBootCore::IClientToBooter& remoteServer );
        QString InfoTask() const;

    // IProtoObserver:
    private:
        void RespCmd(const BfBootCore::UserResponseInfo& code);
    public:        
        TaskSetFactory(
            ITaskOwner& callback,
            dword hwNum,
            dword hwType,
            std::string mac
            );
    };

    // ------------------------------------------------------------------------------------

    class TaskDeleteAllImg : public TaskBase
    {
    // TaskBase override:
    private:
        void RunTask( BfBootCore::IClientToBooter& remoteServer );
        QString InfoTask() const;

        void RespCmd(const BfBootCore::UserResponseInfo& resp);
        
    public:
        TaskDeleteAllImg(ITaskOwner& callback) : TaskBase(callback)
        {}
    };

     
    // ------------------------------------------------------------------------------------

    class TaskCloseTransaction : public TaskBase
    {   
        dword m_softwareReleaseNumber;

    // TaskBase override:
    private:

        void RunTask( BfBootCore::IClientToBooter& remoteServer );
        QString InfoTask() const;

    // IProtoObserver
    private:
        void RespCmd(const BfBootCore::UserResponseInfo& resp);

    public:
        TaskCloseTransaction(ITaskOwner& callback, dword softwareReleaseNumber);
    };

    // ------------------------------------------------------------------------------------

    class TaskLogout : public TaskBase
    {
        bool m_withSave;

    // TaskBase override:
    private:
        void RunTask( BfBootCore::IClientToBooter& remoteServer );
        QString InfoTask() const;

    // IProtoObserver
    private:
        void RespCmd(const BfBootCore::UserResponseInfo& resp);

    public:
        TaskLogout(ITaskOwner& callback, bool withSave);
    };           

    // ------------------------------------------------------------------------------------

    //class TaskEraseDefaultCfg : public TaskBase
    //{
    //// TaskBase override:    
    //private:
    //    void RunTask( BfBootCore::IClientToBooter& remoteServer );
    //    QString InfoTask() const;

    //// IProtoObserver impl:
    //private:
    //    void RespCmd(const BfBootCore::UserResponseInfo& resp);

    //public:
    //    TaskEraseDefaultCfg(ITaskOwner& callback);
    //};

    // ------------------------------------------------------------------------------------

    class TaskSetTrace : public TaskBase
    {
        bool m_traceOn;

    // TaskBase override:    
    private:
        void RunTask( BfBootCore::IClientToBooter& remoteServer );
        QString InfoTask() const;

    // IProtoObserver impl:
    private:
        void RespCmd(const BfBootCore::UserResponseInfo& resp);

    public:
        TaskSetTrace(ITaskOwner& callback, bool traceOn);
    };

    // ------------------------------------------------------------------------------------

    class TaskWrite : public TaskBase
    {   
        enum {CInfoTimeoutMsec = 5 * 1000};
    public:
        enum WriteImgAs {BootImg, AppImg};
        TaskWrite(ITaskOwner& callback, const BfBootCore::IHwFirmwareImage& img, WriteImgAs imgType);            

    // TaskBase override:
    private:

        void RunTask( BfBootCore::IClientToBooter& remoteServer );
        QString InfoTask() const;
    
    // IProtoObserver
    private:
        void RespCmd(const BfBootCore::UserResponseInfo& resp);
        bool WriteChunk();
        void RespNewImgAdded(const BfBootCore::ImgDescriptor& descr);
        void StateInfo(const BfBootCore::SrvStateInfo& info, Platform::int32 progress);

    private:
        enum State {st_null, st_waitOpen, st_opened, st_waitAdded, st_waitCRC, st_waitInfoSaveComplete};
        QString ResolveState() const;
        int Progress() const;   
        void OnInfoTimeout(iCore::MsgTimer*);

        const BfBootCore::IHwFirmwareImage& m_img; // SafeRef -- ?        
        int m_offset;        
        State m_state;

        BfBootCore::IClientToBooter* m_pRemoteServer;
        WriteImgAs m_imgType;  
        
        Utils::TimerTicks m_timer;        
        iCore::MsgTimer m_progTimeout;
        int m_progInd;
    };   

    // ------------------------------------------------------------------------------------

    class TaskRunAddScript : public TaskBase
    {        
        std::string m_scriptName;
        bool m_withSave;

        // TaskBase override:    
    private:
        void RunTask( BfBootCore::IClientToBooter& remoteServer );
        QString InfoTask() const;

    // IProtoObserver impl:
    private:
        void RespCmd(const BfBootCore::UserResponseInfo& resp);

    public:
        TaskRunAddScript(ITaskOwner& callback, const std::string& scriptName, bool withSave = false);
    };

    // ------------------------------------------------------------------------------------

    using BfBootCore::ConfigLine;
    using boost::shared_ptr;

    class TaskViewParams : public TaskBase
    {
        bool m_waitList;
        shared_ptr<std::vector<BfBootCore::ConfigLine> > m_pResult;

    // IProtoObserver
    private:        
        void RespGetParam(const std::vector<BfBootCore::ConfigLine>& userParams);

    // TaskBase override:    
    private:
        void RunTask( BfBootCore::IClientToBooter& remoteServer );
        QString InfoTask() const;

    public:
        TaskViewParams(ITaskOwner& callback, shared_ptr<std::vector<ConfigLine> > pResult);
    };

} // namespace BfBootCli

#endif
