#ifndef __NAMEDOBJECTTESTTYPE__
#define __NAMEDOBJECTTESTTYPE__

#include "Utils/Random.h"
#include "Utils/IVirtualDestroy.h"

#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"

#include "DRI/INonCreatable.h"
#include "DRI/driutils.h"
#include "DRI/NamedObjectFactory.h"
#include "DRI/ISessionCmdContext.h"

#include "iCore/MsgTimer.h"

namespace DriTests
{
    using Domain::NamedObject;
    using Domain::IDomain;
    using Domain::ObjectName;

    class NObjTestType;

    // -----------------------------------------------------------

    class NObjTestTypeInner : 
        public NamedObject, 
        public DRI::INonCreatable
    {
        Q_OBJECT;

    public:

        enum NumberState { Even, Odd };

        enum Binary
        {
            Bit0 = 1,
            Bit1 = 2,
            Bit2 = 4,
            Bit3 = 8,
        };

        Q_ENUMS(NumberState Binary);
        Q_FLAGS(Binaries);

        Q_DECLARE_FLAGS(Binaries, Binary);

    public:

        NObjTestTypeInner(NObjTestType *pParent, const ObjectName &name, int number);

        ~NObjTestTypeInner();

        void AsyncDelete()
        {
            m_parentReqDelete = true;
            MsgObject::AsyncDeleteSelf();
        }

    public:

        Q_PROPERTY(int Number READ m_number WRITE Number);

        Q_PROPERTY(bool IsUserUpdated READ m_isUserUpdated);
        Q_PROPERTY(double SqrtNumber READ m_sqrtNumber);
        Q_PROPERTY(QString NumberAsString READ m_numberAsString);
        Q_PROPERTY(NumberState NumState READ m_numState);
        Q_PROPERTY(Binaries Binary READ m_binary);

        void Number(int val)  { Number(val, true); }


    public:

        Q_INVOKABLE void Inc()
        {
            Number(m_number + 1, true);
        }

        Q_INVOKABLE void Dec()
        {
            Number(m_number - 1, true);
        }

    private:

        NObjTestType *m_pParent;
        int m_number;
        bool m_parentReqDelete;

        bool m_isUserUpdated;
        double m_sqrtNumber;
        QString m_numberAsString;
        NumberState m_numState;
        Binaries m_binary;

        void Number(int val, bool byUser) 
        { 
            m_number = val; 
            m_isUserUpdated = byUser;

            // update other fields
            m_sqrtNumber = std::sqrt((double)m_number);
            m_numberAsString = QString::number(m_number);
            m_numState = (m_number & 1) ? Odd : Even;

            // set binary
            if (m_number & Bit0) m_binary |= Bit0;
            if (m_number & Bit1) m_binary |= Bit1;
            if (m_number & Bit2) m_binary |= Bit2;
            if (m_number & Bit3) m_binary |= Bit3;            
        }

    };

    Q_DECLARE_OPERATORS_FOR_FLAGS(NObjTestTypeInner::Binaries);

    // -----------------------------------------------------------
    
    class NObjTestType : public NamedObject
    {
        Q_OBJECT;

        // dublicates NamedObject::m_children, how to do safe delete and sync this?
        std::vector<NObjTestTypeInner*> m_children;
        int m_childrenNameCounter;
        int m_number;

        static QString ChildNameByNumber(int number)
        {
            return QString("Child%1").arg(number);
        }

        bool AllowCreateChild(DRI::INamedObjectType *pType)  // override
        {
            static std::string name = 
                DRI::DriUtils::FormatedTypename(typeid(*this).name(), true);

            return (pType->TypeName() == name);
        }

        class ObjCounter : public Utils::IVirtualDestroy
        {
            int m_number;
        public:
            ObjCounter() : m_number(0) {}
            int Peek() { return m_number++; }
        };
        
    public:

        NObjTestType(IDomain *pDomain, const ObjectName &name) : 
          NamedObject(pDomain, name),
          m_childrenNameCounter(0)
        {
            const char *CName = "NObjTestType";

            Domain::DomainStore &store = pDomain->getDomain().Store();
            if (!store.StoreExists(CName))
            {
                store.Register(CName, new ObjCounter());
            }

            m_number = store.Get<ObjCounter>(CName).Peek();
        }

        void ChildDeleteDone()
        {
            AsyncOutput("OK!");
            AsyncComplete(true);
        }

    public:

        ESS_TYPEDEF(BadNumber);

        Q_PROPERTY(int Number READ m_number WRITE Number);

        void Number(int val)
        {
            if (val < 0) ESS_THROW(BadNumber);
            m_number = val;
        }

        Q_INVOKABLE void AddChild()
        {
            int number = m_childrenNameCounter++;
            QString name = ChildNameByNumber(number);

            // create; auto register in NamedObject::m_children
            NObjTestTypeInner *p = new NObjTestTypeInner(this, name, number);  

            // register in own list
            m_children.push_back(p);
        }

        Q_INVOKABLE void RemoveChild(DRI::IAsyncCmd *pAsyncCmd, int childIndex)
        {
            AsyncBegin(pAsyncCmd);

            QString child = ChildNameByNumber(childIndex);
            NamedObject *p = FindObject(child);
            if (p == 0) 
            {
                AsyncComplete(false, "Child not found!");
                return;
            }

            NObjTestTypeInner *pC = dynamic_cast<NObjTestTypeInner*>(p);
            ESS_ASSERT(pC != 0);

            pC->AsyncDelete();
        }

        Q_INVOKABLE void ParamsFn(DRI::ICmdOutput *pCmd, 
            int first, QString second = "second...", int last = 666)
        {
            pCmd->Add( QString("%1 %2 %3").arg(first).arg(second).arg(last) );
        }

        Q_INVOKABLE void InfiniteAsyncCmd(DRI::IAsyncCmd *pAsyncCmd)
        {
            AsyncBegin(pAsyncCmd);
        }
    };


    // -----------------------------------------------------------

    class NObjString : public NamedObject
    {
        Q_OBJECT;

        QString m_val;

    public:

        NObjString(IDomain *pDomain, const ObjectName &name) : NamedObject(pDomain, name)
        {
            AllowUserChildrenAdd(true);
        }

        Q_PROPERTY(QString Val READ m_val WRITE m_val);

    };

    // -----------------------------------------------------------

    class NObjPropertyTest : public NamedObject
    {
        Q_OBJECT;

    public:

        enum ColorEnum { clBlack, clWhite, clRed, clBlue, clGreen };

        enum Binary
        {
            Bit0 = 1,
            Bit1 = 2,
            Bit2 = 4,
            Bit3 = 8,
            Bit4 = 16,
            Bit5 = 32,
        };

        Q_ENUMS(ColorEnum Binary);
        Q_FLAGS(Binaries);

        Q_DECLARE_FLAGS(Binaries, Binary);

    public:

        NObjPropertyTest(IDomain *pDomain, const ObjectName &name) : 
          NamedObject(pDomain, name)
        {
            m_str = "just string...";
            m_color = clBlack;
            m_bin = 0;
            m_time = QTime::currentTime();
            m_date = QDate::currentDate();
            m_dt = QDateTime::currentDateTime();
        }

        Q_PROPERTY(QString Str READ m_str WRITE m_str);
        Q_PROPERTY(ColorEnum Color READ m_color WRITE m_color);
        Q_PROPERTY(Binaries Bin READ m_bin WRITE m_bin);
        Q_PROPERTY(QTime Time READ m_time WRITE m_time);
        Q_PROPERTY(QDate Date READ m_date WRITE m_date);
        Q_PROPERTY(QDateTime DT READ m_dt WRITE m_dt);

    private:

        QString m_str;
        ColorEnum m_color;
        Binaries m_bin;
        QTime m_time;
        QDate m_date;
        QDateTime m_dt;

    };

    // -----------------------------------------------------------

    class NObjFlushTest : public NamedObject
    {
        Q_OBJECT

		typedef NObjFlushTest T;

        // run time
        QDateTime m_startTime;
        int m_runTimeMs;

        int m_flushTimeoutMs;
        int m_addTimeoutMs;
        iCore::MsgTimer m_flushTimer;
        iCore::MsgTimer m_addTimer;
        
        bool m_isRunning;
        int m_counter;

        bool IsStopTime()
        {
            return (QDateTime::currentDateTime() >= (m_startTime.addMSecs(m_runTimeMs)));
        }
        
        void OnFlushTimer(iCore::MsgTimer *p);
        void OnAddTimer(iCore::MsgTimer *p);
        void Stop(bool ok, const QString &err = "");

        void AbortAsync();
        void AbortAsyncFake();

        void RunInner(DRI::IAsyncCmd *pCmd, int runTimeMs, boost::function<void (void)> abortFn);

    public:
        ESS_TYPEDEF(FlushTestException);
        
    public:
        NObjFlushTest(IDomain *pDomain, const ObjectName &name);
        ~NObjFlushTest();
        
    // DRI
    public:
        Q_PROPERTY(int FlushTimeoutMs READ m_flushTimeoutMs WRITE m_flushTimeoutMs);
        Q_PROPERTY(int AddTimeoutMs READ m_addTimeoutMs WRITE m_addTimeoutMs);
        
        Q_INVOKABLE void Run(DRI::IAsyncCmd *pCmd, int runTimeMs);
        Q_INVOKABLE void RunWithFakeAbort(DRI::IAsyncCmd *pCmd, int runTimeMs);
        Q_INVOKABLE void RunWithError(DRI::IAsyncCmd *pCmd);
        Q_INVOKABLE void RunInfinite(DRI::IAsyncCmd *pCmd);
        Q_INVOKABLE void RunWithAsyncException(DRI::IAsyncCmd *pCmd);
        Q_INVOKABLE void RunSafeWithAsyncException(DRI::IAsyncCmd *pCmd);
        Q_INVOKABLE void RunWithSyncException(DRI::IAsyncCmd *pCmd);
    };
        
}  // namespace DriTests

#endif
