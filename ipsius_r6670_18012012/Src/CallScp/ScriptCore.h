#pragma once 

#include "Utils/IBasicInterface.h"
#include "ScriptMethodBinder.h"

namespace iCore {   class MsgThread; }
namespace iLogW {   class ILogSessionCreator; }

namespace CallScp
{

    class IExecutionStream : Utils::IBasicInterface
    {
    public:
        virtual const QString& ClassName() const = 0;
        virtual const QString& ObjectName() const = 0;
        virtual const QString& EventName() const = 0;
        virtual QString Info() const = 0;
        virtual bool Completed() const = 0;
        virtual void WaitCompleted(IExecutionStream *pOther) = 0;
    };

    class IScriptCoreUser : public Utils::IBasicInterface
    {
    public:
        virtual bool VerifyEventName(const QString &className, const QString &eventName, /* out */ QString &error) = 0;
        virtual void ScriptComplete(const QString &info) = 0;
    };

    class ScriptCore : boost::noncopyable
    {
    public:

        ESS_TYPEDEF(RuntimeError);

        typedef boost::function<void (const QString &)> TraceFn;

        ScriptCore(iCore::MsgThread &thread,
            TraceFn trace,
            IScriptCoreUser &user);

        ~ScriptCore();

        void Run(const QString &scpName, 
            const QStringList &startupClasses);

        void AddCalss(const QString &className, 
            const QStringList &events); 

        void AddCommand(const QString &commandName, ScriptMethodBinder binder);

        void SetHookOnUnknownCommand(ScriptMethodBinder binder);

        enum CommonCmd 
        {
            cPass,
            cPrint,
            cWait,
            cAssert,
            cRun,
            cEnd
        };

        void AddCommand(CommonCmd command);

        void AddCommonCommands()
        {
            AddCommand(cPass);
            AddCommand(cPrint);
            AddCommand(cWait);
            AddCommand(cAssert);
            AddCommand(cRun);
            AddCommand(cEnd);
        }

        void ScheduleMsg(boost::function<void ()> fn, int delayMs = 0);

        void StartStream(const QString &className,
            const QString &objectName,
            const QString &eventName, 
            bool allowEmpty = true);

    private:
        
        class Impl;
        boost::scoped_ptr<Impl> m_impl;

    };
    
}  // namespace CallScp

