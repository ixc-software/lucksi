#ifndef NOBJBOOTERAUTOUPDATER_H
#define NOBJBOOTERAUTOUPDATER_H

#include "NObjHwFinder.h"
#include "IpsiusService/BootImage.h"
#include "iLog/LogWrapper.h"

namespace IpsiusService
{   
   
    //“ребует предварительного? инстанцировани€ HwFinder

    using boost::scoped_ptr;

    class NObjBooterAutoUpdater 
        : public Domain::NamedObject,
        public IBootUpdater,
        public BfBootCli::IBroadcastReceiverToOwner
    {   
        Q_OBJECT;
        SafeRef<NObjHwFinder> m_finder;
        iCore::MsgTimer m_accumTimer;
        iCore::MsgTimer m_taskProcess;
        int m_minVer;
        QString m_imgFile;

        scoped_ptr<BfBootCli::BroadcastReceiver> m_receiver;
        
        class TaskUpdate;
        Utils::ManagedList<TaskUpdate> m_tasks;
        scoped_ptr<IpsiusService::BootImage> m_bootImg;                 

    // IBootUpdater impl:
    private:        
        void Update(const BfBootCore::BroadcastMsg& msg);        

    // IBroadcastReceiverToOwner
    private:
        void MsgReceived(const BfBootCore::BroadcastMsg& msg);

    private:     
        void ResetDevice(const BfBootCore::BroadcastMsg& msg);
        void TaskFinished(const TaskUpdate* from, const BfBootCli::TaskResult& result);
        void AllFinished(bool ok, QString info = "");  
        void OnDelMsg(const TaskUpdate* ){}
        void LogDri(QString line, bool lf = true);
        void AccumPeriodFinished(iCore::MsgTimer* p);
        void Process(iCore::MsgTimer* p);

    public:
        NObjBooterAutoUpdater(Domain::IDomain *pDomain, const Domain::ObjectName &name);

        ~NObjBooterAutoUpdater();

        Q_INVOKABLE void Run(DRI::IAsyncCmd* pCmd, int minVer, int accumMsec);
        Q_PROPERTY(QString BootIamge READ m_imgFile WRITE m_imgFile);        
    };
} // namespace IpsiusService


#endif
