#ifndef NOBJTASKSETPARAM_H
#define NOBJTASKSETPARAM_H

#include "NObjCommonBfTaskProfile.h"
#include "BfBootCore/ServerTuneParametrs.h"

#include "BfBootCli/ITaskManager.h"
#include "BfBootCli/TaskComposer.h"

#include "NObjBroadcastReceiver.h"
#include "ProfileHolder.h"



using boost::scoped_ptr;
using boost::shared_ptr;

namespace BfBootDRI
{
    class NObjBfTaskSetParams 
        : public Domain::NamedObject,
        public ITask
    {        
        Q_OBJECT
    // DRI interface:
    public:
        NObjBfTaskSetParams(Domain::IDomain *pDomain, const Domain::ObjectName &name);        

        Q_INVOKABLE void Init(QString profile);        

        // Ётого свойства не должнобыть у конечного пользовател€ (использовать макрос ?)
        Q_PROPERTY(bool ResetFactoryIfExist WRITE m_useAutoReset READ m_useAutoReset);

        // ѕлата должна находитс€ в сервисном режиме!
        // ≈сли не вызывать метод то выполн€тс€ только пользовательские настройки
        Q_INVOKABLE void SetFactoryParams(int hwType, int hwNum, QString mac);
        // опциональные параметры
        Q_INVOKABLE void SetOptUdpLogAddr(QString val);                
        Q_INVOKABLE void SetOptUdpLogPort(int port);
        Q_INVOKABLE void SetOptUseTimestamp(bool use);
        Q_INVOKABLE void SetOptCountToStore(int count);
        
        Q_INVOKABLE void SetOptNetwork(bool use, QString ip, QString gateway, QString mask);

        Q_INVOKABLE void SetOptUserPwd(QString pwd = ""); // default = "" is fix surround %_N% if _N is empty
        Q_INVOKABLE void SetOptWaitLoginMsec(int msec);
        Q_INVOKABLE void SetOptCmpAppPort(int port);              
        Q_INVOKABLE void Run(DRI::IAsyncCmd *pAsyncCmd);         

        Q_PROPERTY(bool CheckAfter WRITE m_checkAfter READ m_checkAfter);

    // ITask
    private:
        void Run(bool& isLastTask);
        void RunNext();
    private:    
        NObjBroadcastReceiver* CreateReceiverForValidation();        
        bool ExtractParamsByName(const std::string& name, int& res);
        void OnRunNext();

        ProfileHolder m_prof;     
        scoped_ptr<BfBootCore::DefaultParam> m_factorySettings;        
        BfBootCore::UserParams m_optSettings;
        shared_ptr<BfBootCli::ITaskManager> m_task;       
        bool m_useAutoReset;
        bool m_checkAfter;

        boost::shared_ptr<std::vector<BfBootCore::ConfigLine> > m_waitBoardParams;
        bool m_waitView;
    };
} // namespace BfBootDRI

#endif
