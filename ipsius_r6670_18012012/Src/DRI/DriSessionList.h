#ifndef __DRISESSIONLIST__
#define __DRISESSIONLIST__

#include "Utils/ManagedList.h"
#include "Utils/WeakRef.h"

#include "DriSec/SecUtils.h"

#include "SessionDRI.h"

namespace DRI
{

    typedef boost::function<void ()> CleanupReqFn;

    // This is proxy from ISessionDriOwner to WeakRef
    class DriSessionHolder : public ISessionDriOwner
    {
        Domain::IDomain &m_domain;
        Utils::WeakRef<ISessionDriOwner&> m_owner;
        CleanupReqFn m_cleanupFn;

        const QString m_login;
        QString m_profileName;

        boost::scoped_ptr<SessionDRI> m_session;


        bool VerifyOwner()
        {
            if (m_owner.Empty()) 
            {
                m_cleanupFn();
                return false;
            }

            return true;
        }

        static Utils::SafeRef<DriSec::ISecurity> GetSecurity(Domain::IDomain &domain,
            const QString &login, /* out */ QString &profileName);

    // ISessionDriOwner impl (proxy)
    private:

        void CommandQueueEmpty()
        {
            if (!VerifyOwner()) return;

            m_owner.Value().CommandQueueEmpty();
        }

        void TransactionBeginExec() 
        {
            if (!VerifyOwner()) return;

            m_owner.Value().TransactionBeginExec();
        }

        void TransactionEndExec()
        {
            if (!VerifyOwner()) return;

            m_owner.Value().TransactionEndExec();
        }

        void SessionExitRequest()
        {
            if (!VerifyOwner()) return;

            m_owner.Value().SessionExitRequest();
        }

        void CommandStarted(shared_ptr<CommandDRI> cmd)
        {
            if (!VerifyOwner()) return;

            m_owner.Value().CommandStarted(cmd);
        }

        void CommandOutput(CommandDRI::CommandType type, QString data, bool lineFeed)
        {
            if (!VerifyOwner()) return;

            m_owner.Value().CommandOutput(type, data, lineFeed);
        }

        void CommandCompleted(shared_ptr<CommandDRI> cmd)
        {
            if (!VerifyOwner()) return;

            m_owner.Value().CommandCompleted(cmd);
        }

        QString GetFullSessionInfo() const
        {
            return Info();
        }


    public:

        DriSessionHolder(Domain::IDomain &domain, 
            Utils::WeakRef<ISessionDriOwner&> &owner,
            CleanupReqFn cleanupFn,
            const QString &login,            
            iLogW::ILogSessionCreator &logCreator, int id) :
            m_domain(domain),
            m_owner(owner),
            m_cleanupFn(cleanupFn),
            m_login(login)
        {
            m_session.reset( 
                new SessionDRI( 
                m_domain, *this, logCreator, id, 
                GetSecurity(domain, login, m_profileName) ) 
                );
        }

        ~DriSessionHolder()
        {
            ESS_ASSERT( Lost() );
        }

        bool ReadyToDestroy()
        {
            return m_owner.Empty() && 
                   !m_session->HasTransactions(); 
        }

        bool Lost() const
        {
            return m_owner.Empty();
        }

        bool Active() const
        {
            return m_session->Active();
        }

        SessionDRI& Session()
        {
            return *m_session;
        }

        QString Info() const
        {
            QString s;

            s += QString("User: %1 as %2; ").arg(m_login).arg(m_profileName);

            if ( Lost() )
            {
                s += "LOST";
            }
            else
            {
                s += m_owner.Value().GetFullSessionInfo();
            }

            QString trInfo = m_session->CurrTransactionInfo();
            if ( !trInfo.isEmpty() ) s += "; " + trInfo;

            return s;
        }

        bool LoggedAs(const QString &login)
        {
            return (m_login == login);
        }

    };

    // -------------------------------------------------------
    
    class DriSessionList : public iCore::MsgObject
    {
        typedef DriSessionList T;

        Domain::IDomain &m_domain;
        Utils::ManagedList<DriSessionHolder> m_sessions;
        iCore::MsgTimer m_timer;

        boost::scoped_ptr<iLogW::LogSession> m_log;
        iLogW::LogRecordTag m_tagInfo;

        void DoCleanup()
        {
            int i = 0;

            while(i < m_sessions.Size())
            {
                if (m_sessions[i]->ReadyToDestroy())
                {
                    m_sessions.Delete(i);
                    continue;   // process same index again 
                }

                ++i;
            }
        }

        void CleanupReq()
        {
            PutMsg(this, &T::DoCleanup);
        }

        void OnTimer(iCore::MsgTimer*)
        {
            DoCleanup();
        }

        enum SelectMode { smAll, smLost, smActive };

        int SessionsInfo(SelectMode mode, QString *pInfo = 0) const
        {
            if (pInfo) pInfo->clear();
            int count = 0;

            for(int i = 0; i < m_sessions.Size(); ++i)
            {
                if ( (mode == smLost)   && !m_sessions[i]->Lost() ) continue;
                if ( (mode == smActive) && !m_sessions[i]->Active() ) continue;

                ++count;

                if (pInfo) *pInfo += m_sessions[i]->Info() + "\n";
            }

            return count;
        }

    public:

        DriSessionList(Domain::IDomain &domain) : 
          iCore::MsgObject( domain.getDomain().getMsgThread() ),
          m_domain(domain),
          m_timer(this, &T::OnTimer)
        {
            {                
                m_log.reset( 
                    m_domain.getDomain().Log().CreateSession("Sessions", false)
                    );

                m_tagInfo = m_log->RegisterRecordKindStr("Info");
            }

            m_timer.Start(1000, true);
        }

        ~DriSessionList()
        {
            ESS_ASSERT( m_sessions.IsEmpty() );
        }

        void Shutdown()
        {
            DoCleanup();

            if ( m_sessions.IsEmpty() ) return;

            // halt
            QString msg = "DriSessionList shutdown fail.";
            msg += "\n" + GetDetailedInfo();
            ESS_HALT( msg.toStdString() );
        }

        /*
        void Cleanup()
        {
            DoCleanup();
        }

        void ClearAllSessions()
        {
            m_sessions.Clear();
        } 

        bool Empty() const
        {
            return m_sessions.IsEmpty();
        } */

        SessionDRI& GetNewSession(const QString &login,
            Utils::WeakRef<ISessionDriOwner&> &owner,
            int id)
        {
            DriSessionHolder *p = 
                new DriSessionHolder(m_domain, owner, 
                boost::bind(&T::CleanupReq, this), 
                login, *m_log, id);

            m_sessions.Add(p);

            return p->Session();
        }

        QString GetBriefInfo() const
        {
            int total = m_sessions.Size();
            int lost = SessionsInfo(smLost);

            if (total == 0) return "No sessions";

            QString s = QString::number(total);
            if (lost) s += QString(" (lost %1)").arg(lost);

            return s;
        }

        QString GetDetailedInfo() const 
        {
            QString s;
            SessionsInfo(smAll, &s);
            return s;
        }

        QString GetActiveSessionsInfo() const 
        {
            QString s;
            SessionsInfo(smActive, &s);
            return s;
        }

        iLogW::LogSession& Log()
        {
            return *m_log;
        }

        int LoggedCount(const QString &login)
        {
            int count = 0;

            for(int i = 0; i < m_sessions.Size(); ++i)
            {
                if ( m_sessions[i]->LoggedAs(login) ) ++count;
            }

            return count;
        }

                        
    };
    
    
}  // namespace DRI

#endif
