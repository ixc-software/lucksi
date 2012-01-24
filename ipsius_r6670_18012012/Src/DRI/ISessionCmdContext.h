#ifndef __ISESSIONCMDCONTEXT__
#define __ISESSIONCMDCONTEXT__

#include "Utils/IBasicInterface.h"
#include "Utils/SafeRef.h"

namespace Domain
{
    class NamedObject;
}

namespace DriSec
{
    class ISecurity;
}

// ------------------------------------------------

namespace DRI
{

    // special arg classes inherit from this type
    class ISpecialArg : public Utils::IBasicInterface
    {
        // nothing
    };

    // -------------------------------------------------------

    class MetaObjectsInfo;
    
    // CS_xxx commands implementation inside SessionDRI
    // All CS_xxx commands in NamedObject have first param of this type
    class ISessionCmdContext : public ISpecialArg
    {
    public:
        
        virtual void SetCurrentObject(QString absoluteName) = 0;
        virtual void SetCurrentObjectBack() = 0;

        virtual void TransactionBegin(bool continueOnError) = 0;
        virtual void TransactionEnd() = 0;
        virtual void TransactionDiscard() = 0;

        virtual void SetCommandTimeout(int intervalMs) = 0;

        virtual void ExecuteFile(QString fileName, QString macroReplaceList) = 0;

        virtual void OutputPrint(QString msg) = 0;

        virtual void SessionExitRequest() = 0;

        virtual MetaObjectsInfo& MetaObjInfo() = 0; // !const

        virtual void PrintSessionInfo() = 0; 

        virtual void SwitchSecProfile(QString profileName) = 0;

        virtual DriSec::ISecurity* GetSecurity() = 0;
    };

    // -------------------------------------------------------

    // Sync DRI command with output
    class ICmdOutput : public ISpecialArg
    {
    public:
        virtual void Add(const QString &data, bool lineFeed) = 0;
        virtual DriSec::ISecurity* GetSecurity() = 0;

        void Add(const QString &data)
        {
            Add(data, true);
        }

        void Add(const QStringList &sl)
        {
            for(int i = 0; i < sl.size(); ++i) Add(sl.at(i), true);
        }

        virtual void Flush() = 0;

    };

    // -------------------------------------------------------

    // Async DRI command with output
    class IAsyncCmd : public ICmdOutput
    {
    public:
        virtual void AsyncCompleted(bool isOK, const QString &errorMsg = "") = 0;
        virtual void AsyncStarted(Utils::SafeRef<Domain::NamedObject> asyncObject) = 0;
    };
        
}  // namespace DRI


#endif

