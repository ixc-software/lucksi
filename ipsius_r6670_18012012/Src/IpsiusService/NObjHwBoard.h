#ifndef __NOBJHWBOARD__
#define __NOBJHWBOARD__

#include "Utils/IntToString.h"
#include "Utils/IBasicInterface.h"
#include "Utils/QtEnumResolver.h"
#include "Utils/VirtualInvoke.h"

#include "iCore/MsgThread.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "iLog/LogWrapper.h"
#include "DRI/INonCreatable.h"
#include "BfBootCore/BroadcastMsg.h"
#include "BfBootCli/ITaskManager.h"
#include "BfBootCli/ITaskManagerReport.h"
#include "BfBootCore/GeneralBooterConst.h"

#include "HwFirmwareStore.h"


namespace IpsiusService
{
    using Domain::NamedObject;
    using Domain::IDomain;
    using Domain::ObjectName;    

    // ----------------------------------------------------

    class NObjHwBoard;

    class IHwBoardOwner : public Utils::IBasicInterface
    {
    public:
        virtual boost::shared_ptr<HwFirmware> FirmwareRequest(const BfBootCore::BroadcastMsg &msg) const = 0;

        virtual void BoardEnterReadyState(const NObjHwBoard &board) = 0;
        virtual void BoardLeaveReadyState(const NObjHwBoard &board) = 0;

        virtual QString GetBoardOwner(const NObjHwBoard &board) const = 0;
        virtual void ResetBoard(const BfBootCore::BroadcastMsg &msg) const = 0;        

        virtual bool BoardTraceEnabled() const = 0;
        virtual void BoardTrace(QString src, QString msg) = 0;
        virtual bool TaskMngTraceEnabled() const = 0;
		virtual bool TraceProgress() const = 0;

        virtual bool IsFiltered(const BfBootCore::BroadcastMsg &msg) const = 0;
        virtual bool IsDirect(const BfBootCore::BroadcastMsg &msg) const = 0;
    };

    // ----------------------------------------------------        

    // обнаруженная (через броадкаст) плата
    class NObjHwBoard :
        public NamedObject, 
        public DRI::INonCreatable
    {      
        Q_OBJECT;
    public:        
        NObjHwBoard(Domain::NamedObject *pParent, IHwBoardOwner &owner, const BfBootCore::BroadcastMsg &msg);                   
        ~NObjHwBoard();
        bool ProcessMsg(const BfBootCore::BroadcastMsg &msg); 
        void TimeProcess();
        bool CleanRequired() const;
        int HwNumber() const;
        const BfBootCore::BroadcastMsg& Msg() const;
        bool IsFiltered() const;
        bool IsDirect() const;

    // DRI meta
    public:
        Q_PROPERTY( int HwNumber READ getHwNumber );
        Q_PROPERTY( int HwID READ getHwID );
        Q_PROPERTY( QString IP READ getIP );
        Q_PROPERTY( QString MAC READ getMAC );
        Q_PROPERTY( int BooterPort READ getBooterPort );
        Q_PROPERTY( int CmpPort READ getCmpPort );
        Q_PROPERTY( int BootRelease READ getBootRelease );
        Q_PROPERTY( QString BootReleaseInfo READ getBootReleaseInfo );
        Q_PROPERTY( int SoftRelease READ getSoftRelease );
        Q_PROPERTY( QString SoftReleaseInfo READ getSoftReleaseInfo );

        Q_PROPERTY( QString State READ getState );
        Q_PROPERTY( QString StateDesc READ getStateDesc );
        Q_PROPERTY( QString Owner READ getOwner );
        Q_PROPERTY( QDateTime FirstRecv READ getFirstRecv );
        Q_PROPERTY( QDateTime LastRecv READ getLastRecv );
        Q_PROPERTY( int DiscoveredCount READ m_discovered );
        Q_PROPERTY( int BroadcastCounter READ m_broadcastCounter );
        Q_PROPERTY( QDateTime StateEnter READ getStateEnter );
        Q_PROPERTY( bool Filtered READ IsFiltered);

        Q_INVOKABLE void ListInfo(DRI::ICmdOutput *pCmd, bool briefly) const;    

    // DRI getters
    private:
        int getHwNumber() const;
        int getHwID() const;
        QString getIP() const;
        QString getMAC() const;
        int getBooterPort() const;
        int getCmpPort() const;
        int getBootRelease() const;  
        QString getBootReleaseInfo() const;
        int getSoftRelease() const;
        QString getSoftReleaseInfo() const;  
        QString getStateDesc() const; 
        QString getOwner() const;         
        QDateTime getFirstRecv() const;
        QDateTime getLastRecv() const;
        QDateTime getStateEnter() const;
        QString getState() const;

    private:
        void InkBroadcastCounter() { ++m_broadcastCounter; }
        void InkConflictCounter() { ++m_conflictsCount; }
        void InkDiscoverCounter();        

    private:        
        class HwBoardFsm;
        IHwBoardOwner &m_owner;                
		boost::scoped_ptr<iLogW::LogSession> m_log;                        
        QStringList m_shortPropList;
        QStringList m_longPropList;        
        boost::scoped_ptr<HwBoardFsm> m_fsm;

        // counters
        int m_discovered;
        int m_conflictsCount;
        int m_broadcastCounter;
    };    
    
}  // namespace IpsiusService

#endif
