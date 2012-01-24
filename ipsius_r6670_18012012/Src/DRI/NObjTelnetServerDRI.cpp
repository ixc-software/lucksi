
#include "stdafx.h"

#include "NObjTelnetServerDRI.h"
#include "TelnetServerSessionDRI.h"
#include "Utils/QtHelpers.h"

#include "Domain/DomainClass.h"


namespace DRI
{
    NObjTelnetServerSessionDRI::NObjTelnetServerSessionDRI(Domain::NamedObject *pParent, 
                                                       const Domain::ObjectName &name, 
                                                       ITelnetSessionDRIToServer &owner, 
                                                       shared_ptr<iNet::ITcpSocket> socket) :
        NamedObject(&pParent->getDomain(), name, pParent),
        m_session( new TelnetServerSessionDRI(*this, owner, Log(), socket) )
	{
    }

    // ----------------------------------------------------------------

    NObjTelnetServerSessionDRI::~NObjTelnetServerSessionDRI()
    {
    }

    // ----------------------------------------------------------------
    
    void NObjTelnetServerSessionDRI::Close()
    {
        m_session->Close();
    }

    // ----------------------------------------------------------------

    bool NObjTelnetServerSessionDRI::IsSame(const TelnetServerSessionDRI *pSession) const
    {
        return m_session->IsSame(pSession);
    }

    // ----------------------------------------------------------------

    /*
    namespace 
    {
        template<class Tag, class Val>
			QString Line(const Tag &tag, const Val &val)
        {
            return QString("%1 = %2").arg(tag).arg(val);
        }
    
    } // namespace 
    
    QString NObjTelnetServerSessionDRI::ToString() const
    {
        QStringList res;
        res << Line("Type", "Telnet") 
            << Line("Name", Name().GetShortName())
            << Line("LocalPort", QString::number(m_localPort))
            << Line("RemoteAddress", RemoteAddress())
            << Line("StartTime", StartTime().toString("dd.MM.yyyy hh:mm:ss"))
            << Line("Statistic", Statistic())
            << Line("LoginName", LoginName())
			<< Line("LogLevel", LogLevel());

        return res.join("\n");
    } */

    // ----------------------------------------------------------------
    // NObjTelnetSessionDriInfoList impl
    
    NObjTelnetServerDRI::NObjTelnetServerDRI(Domain::NamedObject *pParent, 
                                             const Domain::ObjectName &name) :
        NamedObject(&pParent->getDomain(), name, pParent),
        m_domain(*pParent), 
        m_server(pParent->getDomain().getMsgThread(), 
                 this, 
                 pParent->getDomain().Params().TelnetDriAddr()),
        m_allSessionCounter(0),
        m_closeDomainIfNoSessions(false)
    {}

    // ----------------------------------------------------------------

    NObjTelnetServerDRI::~NObjTelnetServerDRI()
    {
    }

    // ----------------------------------------------------------------

    void NObjTelnetServerDRI::CloseAllSessions()
    {
        for (size_t i = 0; i < m_sessions.size(); ++i)
        {
            m_sessions.at(i)->Close();

        }

        // session info owner unregister self from destructor
    }
    
    // ----------------------------------------------------------------

    /*
    QString NObjTelnetServerDRI::FindSessionInfo(const QString &name) const
    {
        int index = Find(name);
        ESS_ASSERT(index >= 0);

        return m_sessions.at(index)->ToString();
    } 
    
    void NObjTelnetServerDRI::CloseSession(const QString &name)
    {
        int index = Find(name);
        ESS_ASSERT(index >= 0);

        m_sessions.at(index)->Close();        
    }
        
    */

    // ----------------------------------------------------------------
    
    int NObjTelnetServerDRI::Find(const QString &name) const
    {
        ESS_ASSERT(!name.isEmpty());
        
        for (size_t i = 0; i < m_sessions.size(); ++i)
        {
            if (m_sessions.at(i)->Name() == name) return i;
        }

        return -1;
    }

    // ----------------------------------------------------------------
    
    int NObjTelnetServerDRI::Find(const TelnetServerSessionDRI *pSession) const
    {
        for (int i = 0; i < m_sessions.size(); ++i)
        {
            if (m_sessions.at(i)->IsSame(pSession)) return i;
        }

        return -1;
    }

    // ----------------------------------------------------------------

    Domain::IDomain& NObjTelnetServerDRI::Domain()
    {
        return m_domain.getDomain();
    }

    // ----------------------------------------------------------------

    iCore::MsgThread& NObjTelnetServerDRI::MsgThread()
    {
        return m_domain.getDomain().getMsgThread();
    }

    // ----------------------------------------------------------------

    void NObjTelnetServerDRI::Unregister(const TelnetServerSessionDRI *pSession)
    {
        ESS_ASSERT( Find(pSession) >= 0 );

        // session must be deleted async'ed 'couse we are inside callback from it
        // session don't be deleted as message 'couse it's cleared as NamedObject in m_sessions
        PutMsg(this, &NObjTelnetServerDRI::OnUnregister, pSession, true);
                
        // m_domain.getDomain().getMsgThread().AsyncDelete( m_sessions.Detach(indx) );
    }

    // ----------------------------------------------------------------

    void NObjTelnetServerDRI::OnUnregister(const TelnetServerSessionDRI *pSession)
    {
        int index = Find(pSession);
        ESS_ASSERT(index >= 0);
        
        delete m_sessions.at(index);
        m_sessions.erase(m_sessions.begin() + index);

        if (m_sessions.size() == 0 && m_closeDomainIfNoSessions)
        {
            m_domain.getDomain().Stop(Domain::DomainExitEsc);
        }
    }

    // ----------------------------------------------------------------

    void NObjTelnetServerDRI::ServerNewClientConnectInd(
                                            boost::shared_ptr<iNet::ITcpSocket> socket)
    {
        ++m_allSessionCounter;
        QString name(QString("Session%1").arg(m_allSessionCounter));
        
        m_sessions.push_back(
			new NObjTelnetServerSessionDRI(this, name, *this, socket));        
    }

    // ----------------------------------------------------------------

    void NObjTelnetServerDRI::ServerSocketErrorInd(
                                            boost::shared_ptr<iNet::SocketError> error)
    {
        // ESS_HALT( error->getErrorString().toStdString() );
        ESS_THROW_MSG(SocketError, error->getErrorString().toStdString());
    }
        
} // namespace DRI
