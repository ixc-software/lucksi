
#ifndef __NOBJTELNETSERVERDRI__
#define __NOBJTELNETSERVERDRI__


#include "ITelnetDRI.h"
#include "INonCreatable.h"

#include "Domain/NamedObject.h"

#include "Utils/HostInf.h"
#include "Utils/ManagedList.h"
#include "iLog/LogWrapper.h"
#include "iNet/ITcpSocket.h"

#include "Telnet/ITelnet.h"
#include "Telnet/TelnetServer.h"

namespace DRI
{
    class TelnetServerSessionDRI;

    // Using to operate Telnet session from within session
    class NObjTelnetServerSessionDRI : 
        public Domain::NamedObject,
        public INonCreatable
    {
        Q_OBJECT;

        boost::scoped_ptr<ITelnetSessionDRIToOwner> m_session;

        /*
        int m_localPort;
        Utils::HostInf m_remoteAddr;
        QDateTime m_startTime;
        QString m_stat; */

        QString GetInfo()
        {
            return m_session->GetInfo();
        }

    // DRI setters/getters
    private:

        /*
        int LocalPort() const { return m_localPort; }
        QString RemoteAddress() const { return QString::fromStdString(m_remoteAddr.ToString()); }
        QDateTime StartTime() const { return m_startTime; }
        QString Statistic() const;
        QString LoginName() const;
        bool IsTraceEnabled() const;
        QString GetShortName() const; */

    // DRI interface
    public:

        Q_PROPERTY(QString Info READ GetInfo);

        /*
        Q_PROPERTY(QString Name READ GetShortName);
        Q_PROPERTY(int LocalPort READ LocalPort);
        Q_PROPERTY(QString RemoteAddress READ RemoteAddress);
        Q_PROPERTY(QDateTime StartTime READ StartTime);
        Q_PROPERTY(QString Statistic READ Statistic);
        Q_PROPERTY(QString LoginName READ LoginName); */

        Q_INVOKABLE void Close();

        Q_INVOKABLE void GetStats(DRI::ICmdOutput *pOutput)
        {
            pOutput->Add( m_session->GetStats() );
        }
        
    public:
        NObjTelnetServerSessionDRI(Domain::NamedObject *pParent, 
                                 const Domain::ObjectName &name, 
                                 ITelnetSessionDRIToServer &owner, 
                                 boost::shared_ptr<iNet::ITcpSocket> socket);

        ~NObjTelnetServerSessionDRI();

        bool IsSame(const TelnetServerSessionDRI *pSession) const;
        
        /* 
        void Statistic(const QString &val) { m_stat = val; }
        QString ToString() const; */
    };

    // ----------------------------------------------------------

    // Using to operate Telnet server from within session.
    // Server + list of sessions.
    class NObjTelnetServerDRI : 
        public Domain::NamedObject,
        public Telnet::ITelnetServerEvents,
        public ITelnetSessionDRIToServer,
        public INonCreatable
    {
        Q_OBJECT

        Domain::IDomain &m_domain;
            
        Telnet::TelnetServer m_server;
        std::vector<NObjTelnetServerSessionDRI*> m_sessions; // deleted via NamedObject

        int m_allSessionCounter; // never decreases
        bool m_closeDomainIfNoSessions;

        int Find(const QString &name) const;
        int Find(const TelnetServerSessionDRI *pSession) const;
        void OnUnregister(const TelnetServerSessionDRI *pSession);
        
    // ITelnetSessionDRIToServer impl
    private:

        Domain::IDomain& Domain();
        iCore::MsgThread& MsgThread();
        void Unregister(const TelnetServerSessionDRI *pSession);

    // ITelnetServerEvents impl
    private:
        void ServerNewClientConnectInd(boost::shared_ptr<iNet::ITcpSocket> socket);
        void ServerSocketErrorInd(boost::shared_ptr<iNet::SocketError> error); // can throw

    // DRI interface
    public:
        Q_INVOKABLE void CloseAllSessions();
        
    public:
        ESS_TYPEDEF(SocketError);
        
    public:
        NObjTelnetServerDRI(Domain::NamedObject *pParent, 
                            const Domain::ObjectName &name = "TelnetServer"); 
        ~NObjTelnetServerDRI();

        // QString FindSessionInfo(const QString &name) const;
        // void CloseSession(const QString &name);

        bool CloseDomainIfNoSessions() const { return m_closeDomainIfNoSessions; }
        void CloseDomainIfNoSessions(bool val) { m_closeDomainIfNoSessions = val; }
        
        Utils::HostInf LocalHostInf() const { return m_server.LocalHostInf(); }
    };
    
} // namespace DRI

#endif
