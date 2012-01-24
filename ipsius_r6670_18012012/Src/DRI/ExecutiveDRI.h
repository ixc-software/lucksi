#ifndef __EXECUTIVEDRI__
#define __EXECUTIVEDRI__

#include "Utils/IBasicInterface.h"
#include "Utils/HostInf.h"
#include "iCore/MsgObject.h"
#include "Domain/IDomain.h"

#include "TransactionDRI.h"
#include "DriArgFactory.h"
#include "DriHintDb.h"
#include "DriPropertyFactory.h"
#include "MetaObjectsInfo.h"
#include "DriSpecClassFactory.h"

namespace DRI
{
    // class TelnetServerDRI;

    class DriSessionList;
    class SessionDRI;
    class ISessionDriOwner;

    // -------------------------------------------------------

    // источник DRI транзакций на исполнение (имплементируется сессией)
    class ITransactionSource : public Utils::IBasicInterface
    {
    public:
        virtual TransactionDRI* PeekTransaction() = 0;
        virtual void NotifyTreeChanged() = 0;
    };


    // -------------------------------------------------------
    
    /*
        Класс, ставящий DRI транзакции на исполнение
    */
    class ExecutiveDRI : 
        public iCore::MsgObject,
        boost::noncopyable
    {
        typedef std::vector<ITransactionSource*> TransactionSources;

        Domain::IDomain &m_domain;
        int m_currSessionNumber;
        TransactionSources m_sources;
        DriArgFactory m_argFactory;
        DriHintDb m_hintDb;
        DriPropertyFactory m_propertyFactory;
        DriSpecClassFactory m_specClassFactory;

        boost::scoped_ptr<DriSessionList> m_sessionsList;
        
        TransactionDRI  *m_pActiveTr;

        // boost::scoped_ptr<TelnetServerDRI> m_telnetSrv;

        MetaObjectsInfo m_metaInfo;

        TransactionSources::iterator Find(ITransactionSource *pSrc)
        {
            return std::find(m_sources.begin(), m_sources.end(), pSrc);
        }

        void DoActivate()
        {
            // already busy
            if (m_pActiveTr != 0) return;

            for(size_t i = 0; i < m_sources.size(); ++i)
            {
                m_pActiveTr = m_sources.at(i)->PeekTransaction();
                if (m_pActiveTr != 0) 
                {
                    m_pActiveTr->Execute();
                    break;
                }
            }
        }

        void DoNotifyTreeChanged()
        {
            for(size_t i = 0; i < m_sources.size(); ++i)
            {
                m_sources.at(i)->NotifyTreeChanged();
            }
        }

    public:

        ExecutiveDRI(Domain::IDomain &domain);
        ~ExecutiveDRI();

        void RegisterSource(ITransactionSource *pSrc)
        {
            // dublicates check
            ESS_ASSERT( Find(pSrc) == m_sources.end());

            // add
            m_sources.push_back(pSrc);
            // ESS_ASSERT(Find(pSrc) != m_sources.end());  // debug

            Activate();
        }

        void UnregisterSource(ITransactionSource *pSrc)
        {
            TransactionSources::iterator i = Find(pSrc);
            ESS_ASSERT(i != m_sources.end());

            // remove
            m_sources.erase(i);
        }

        // у одного из источников появилась транзакция, принудительный опрос зарегестрированных источников
        void Activate()
        {
            PutMsg(this, &ExecutiveDRI::DoActivate);
        }

        // после выхода из функции указатель pTransaction недействителен
        void TransactionDone(TransactionDRI *pTransaction)
        {
            ESS_ASSERT(m_pActiveTr == pTransaction);
            m_pActiveTr = 0;

            Activate();
        }
        
        int AllocSessionNumber() 
        {
            return m_currSessionNumber++;
        }

        SessionDRI& GetNewSession(const QString &login, Utils::WeakRef<ISessionDriOwner&> owner, int id);

        DriSessionList& SessionList() const
        {
            ESS_ASSERT(m_sessionsList != 0);
            return *m_sessionsList;
        }

        DriArgFactory& ArgFactory() { return m_argFactory; }
        DriHintDb& HintDB() { return m_hintDb; }
        DriPropertyFactory& PropertyFactory() { return m_propertyFactory; }
        DriSpecClassFactory& SpecClassFactory() { return m_specClassFactory; }
        
        void ObjectTreeChanged(bool objDeleted)
        {            
            m_hintDb.ObjectTreeChanged();

            if (objDeleted) PutMsg(this, &ExecutiveDRI::DoNotifyTreeChanged);
        }

        // Utils::HostInf LocalHostInf() const;

        MetaObjectsInfo& MetaObjInfo();

        // void CloseTelnetServer();
    };
    
}  // namespace DRI

#endif
