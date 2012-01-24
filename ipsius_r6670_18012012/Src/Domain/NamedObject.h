#ifndef __NAMEDOBJECT__
#define __NAMEDOBJECT__

#include "stdafx.h"
#include "Utils/IBasicInterface.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/StringList.h"
#include "Utils/VirtualInvoke.h"
#include "Utils/SafeRef.h"
#include "iCore/MsgObject.h"
#include "iLog/LogWrapper.h"
#include "DRI/ISessionCmdContext.h"

#include "ObjectName.h"
#include "IDomain.h"
#include "IDomainExceptionHook.h"
#include "DRI/INonCreatable.h"


namespace DRI
{

    class INamedObjectType;
    class DriArgFactory;
    class IAsyncCmd;
    class ExecutiveDRI;

}  // namespace DRI

namespace Domain
{

    class NamedObject;

    class IPropertyWriteEvent : public Utils::IBasicInterface
    {
    public:
        virtual bool OnPropertyWrite(NamedObject *pObject, QString propertyName, QString val) = 0;
    };

    // --------------------------------------------------------

    /*
    Именованный объект, т.е. объект, который доступен в рамках домена
    по имени. Каждый объект имеет родителя и некоторое количество
    вложенных объектов -- детей
    */
    class NamedObject 
        : public QObject, 
          public iCore::MsgObject, 
          public virtual IDomain,
          public IPropertyWriteEvent,
          public IDomainExceptionHook,
          // DRI::INonCreatable,
          boost::noncopyable
    {
    public:

        typedef std::list<NamedObject*> ChildrenList;

        /*
            Варианты создания объектов

            1. Единственный экземпляр рутового объекта:
               NamedObject("", 0, true)

            2. Создание объекта с указанием его полного имени, при этом родитель
               ищеться автоматически:
               NamedObject("ObjectParent.ObjectName")

            3. Создание объекта с указанием его короткого имени и непосредственным
               указаением объекта-родителя:
               NamedObject("ObjectName", pParent)

        */
        NamedObject(IDomain *pDomain, const ObjectName &name, NamedObject *pParent = 0, bool isRoot = false);

        const ObjectName& Name() const
        {
            return m_name;
        }

        const ObjectName& NameShort() const
        {
            return m_nameShort;
        }

        FullObjectName GetFullObjectName() const;

        bool IsRoot() const
        {
            return (m_pParent == 0);
        }

        NamedObject* FindObject(const QString &name) const; // this version don't throw on bad object name       
        NamedObject* FindObject(const ObjectName &name) const;        

        QString Property(QString propertyName, bool withMarkRO) const;
        bool Property(QString propertyName, QString val);

        // синхронное (через вызов деструктора) удаление объекта и всех его детей
        void Delete()
        {
            delete this;
        }

        // verify object state before method call or property read/write
        void BeforeInvoke() const;

        // list all types, method, props names (for hint engine)
        void GetAllMetaIdentificators(Utils::StringList &sl) const;

        // get names of all object downside in tree
        void GetAllShortNames(Utils::StringList &sl) const;

        QStringList GetAllChildrenNames() const;

        // property write event handler 
        void PropertyWriteEvent(Utils::SafeRef<IPropertyWriteEvent> handle);
        void PropertyWriteEventAbort(QString msg);
        void PropertyWriteEventClear();

        // Returns true if succeed 
        bool AbortAsyncCommand();

        static QStringList GetAllDriActiveMethodList();

    // IDomain implementation
    public:

        DomainClass& getDomain() { return m_domain; }

    public:

        enum ListPropertiesMode
        {
            lpmNameType,
            lpmNameValue,
            lpmNameTypeValue,
            lpmValue,
        };

	// log level
	public:

		enum LogTag
		{ 
			ErrorTag,
			WarningTag,
			InfoTag
		};
		Q_ENUMS(LogTag);

    protected:

        // Объекты запрещено создавать на стеке, удалять объекты нужно командой Delete()
        virtual ~NamedObject();

        void ThrowRuntimeException(const std::string &msg) const;

        void ThrowRuntimeException(const QString &msg) const
        {
            ThrowRuntimeException(msg.toStdString());
        }

        void ThrowRuntimeException(const char *pMsg) const
        {
            ESS_ASSERT(pMsg != 0);
            ThrowRuntimeException(std::string(pMsg));
        }

        bool AllowUserChildrenAdd() const { return m_allowUserChildrenAdd; }
        void AllowUserChildrenAdd(bool val) { m_allowUserChildrenAdd = val; }

        // Функцию можно перекрыть, для возможности проверки корректности типа
        virtual bool AllowCreateChild(DRI::INamedObjectType *pType)
        {
            return AllowUserChildrenAdd();
        }

        void FreeChildren(int remainsCount = 0);

    protected:

		void AsyncBegin(DRI::IAsyncCmd *pCmd, 
			boost::function<void ()> abortFn = boost::function<void ()>());
        void AsyncOutput(const QString &line, bool lineFeed = true) const;
        void AsyncComplete(bool ok, const QString &errorMsg = "");
        bool AsyncActive() const;
        void AsyncFlush();

        virtual void OnObjectDelete(DRI::IAsyncCmd *pAsyncCmd)
        {
            /* Nothing

               Write customisation code in overrided method; you can:
               - do something sync and usefull
               - throw runtime exception, i.e. object can't be deleted now
               - call AsyncBegin() and begin async delete procedure;
                 complete it with AsyncComplete() + AsyncDeleteSelf()
            */
        }

    protected:

        void EnableThreadHook();
        void DisableThreadHook();

        // override it for custom exception hook
        // return true for suppress exception e 
        virtual bool SuppressDomainExceptionHook(const std::exception &e)
        {
            // nothing, use for override
            return false;
        }

	// log management
	protected:

		iLogW::LogSession& Log();

		iLogW::LogSession& Log(LogTag tag);
		
		bool IsLogActive(LogTag tag); // так же выполняется запись тэга в сессию

		bool IsTagActive(LogTag tag) const;

		int  LogLevel() const;

		bool LogLevel(int level);

		void SetupLogLevelToChild(int level);

    // CS_xxx commands impl
    private:
        
        // особый метод, т.к. применяется в контексте конкретного объекта
        Q_INVOKABLE void CS_Set(DRI::ISessionCmdContext *pContext, 
            QString path = "") const;  

        Q_INVOKABLE void CS_SetBack(DRI::ISessionCmdContext *pContext) const;  

        Q_INVOKABLE void CS_Begin(DRI::ISessionCmdContext *pContext, 
            bool continueOnError = false) const;
        Q_INVOKABLE void CS_End(DRI::ISessionCmdContext *pContext) const;
        Q_INVOKABLE void CS_Discard(DRI::ISessionCmdContext *pContext) const;

        Q_INVOKABLE void CS_Timeout(DRI::ISessionCmdContext *pContext, 
            int intervalMs) const;

        Q_INVOKABLE void CS_Exec(DRI::ISessionCmdContext *pContext, 
            QString fileName, QString macroReplaceList = "") const;

        Q_INVOKABLE void CS_Print(DRI::ISessionCmdContext *pContext, 
            QString text = "") const;

        Q_INVOKABLE void CS_Exit(DRI::ISessionCmdContext *pContext) const;
        
        Q_INVOKABLE void CS_Info(DRI::ISessionCmdContext *pContext) const;

    // Object manipulations
    private:

        Q_INVOKABLE void ObjCreate(DRI::ISessionCmdContext *pContext, 
            QString objectType, QString objectName = "", bool setAsCurrent = false);

        Q_INVOKABLE void ObjDelete(DRI::IAsyncCmd *pAsyncCmd);
        // Q_INVOKABLE void ObjDeleteChildren();  // view notes at method body

        Q_INVOKABLE void ObjList(DRI::ICmdOutput *pOutput, 
            bool listProperties = false, int recurseLevel = 0) const;
        Q_INVOKABLE void ObjView(DRI::ICmdOutput *pOutput, 
            bool withTypes = false, QString propertyName = "") const;

    // Meta commands
    private:

        Q_INVOKABLE void MetaMethods(DRI::ICmdOutput *pOutput, 
            QString typeName = "", QString methodName = "") const;

        Q_INVOKABLE void MetaFields(DRI::ICmdOutput *pOutput,
            QString typeName = "", QString propertyName = "") const;

        Q_INVOKABLE void MetaEnums(DRI::ICmdOutput *pOutput, 
            QString typeName = "", QString enumName = "") const;

        Q_INVOKABLE void MetaTypeInfo(DRI::ICmdOutput *pOutput, 
            QString typeName = "", bool withInherited = false) const;

        Q_INVOKABLE void MetaTypes(DRI::ICmdOutput *pOutput, 
            bool fullInfo = false, bool withInherited = false, bool onlyCreatable = false) const;

    // IPropertyWriteEvent impl
    private:
        
        bool OnPropertyWrite(NamedObject *pObject, QString propertyName, QString val);
	
	// LogLevel 
    private:
	
		Q_PROPERTY(int LogLevel READ LogLevelDri WRITE LogLevelDri);

		void InitLog();
		int  LogLevelDri() const;
		void LogLevelDri(int level);

    // IDomainExceptionHook impl
    private:

        bool DomainExceptionSuppress(const std::exception &e)
        {
            return SuppressDomainExceptionHook(e);
        }



    private:

        Q_OBJECT;

        DomainClass& m_domain;   

        ObjectName m_name;       // полное имя
        ObjectName m_nameShort;  // короткое имя 

        NamedObject *m_pParent; 

        // возможность блокировать выполнение RemoveChild(), указывая, от какого именно
        // объекта мы ожидаем callback 
        NamedObject *m_pWaitChildForDelete;  

        ChildrenList m_children;

        Utils::SafeRef<DRI::IAsyncCmd> m_pAsync;    // pointer to current async command context
        boost::function<void ()> m_asyncAbort;		// custom async abort

        bool m_root; 
        bool m_allowUserChildrenAdd;
        bool m_exceptionHookEnabled;

        Utils::SafeRef<IPropertyWriteEvent> m_onPropertyWrite;
        QString m_propertyWriteAbortMsg;

        void AddChild(NamedObject *pChild);
        void RemoveChild(NamedObject *pChild);
        NamedObject* FindChildByName(const ObjectName &name) const;
        QString GenerateNewName(QString objectType) const;

        DRI::ExecutiveDRI& ExeDRI() const;

        // resolve abs/relative path (+ property name) to string
        QString PathToValue(const QString &path, bool tryTreatAsProperty) const;  // can throw

        bool ReadProperty(const QString &propertyName, QString &result, bool withMarkRO) const;

        void ObjListRecursive(DRI::ICmdOutput *pOutput, bool listProperties, int currLevel, int maxLevel) const;
        QString ObjectNameAndType(int identLevel = 0) const;
        DRI::INamedObjectType* FindNamedObjectType(QString typeName, bool throwException) const;
        const QMetaObject* MetaObjectForType(QString typeName = "", bool throwException = true) const;
        void ParentForChildCreationCheck(QString fullChildName, DRI::INamedObjectType *pType);
        bool IsNonCreatable();
        void PropertyList(DRI::ICmdOutput *pOutput, 
            QString typeName, QString propertyName, ListPropertiesMode mode) const;

        DRI::DriArgFactory& ArgFactory() const;
		class LogImpl;
		boost::scoped_ptr<LogImpl> m_log;

    };

} // namespace Domain


#endif

