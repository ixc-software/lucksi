#include "stdafx.h"

#include "Utils/ErrorsSubsystem.h"
#include "Utils/IniFile.h"
#include "Utils/GetDefLocalHost.h"
#include "Utils/EnvVarsInPath.h"

#include "NObjHwFinder.h"
#include "NObjBoardFilter.h"

// ---------------------------------------

namespace
{   
    void TestIni()
    {
        using namespace Utils;
        using boost::scoped_ptr;

        std::vector<std::string> data;

        scoped_ptr<IniFile> p( IniFile::LoadFromStringContainer(data) );
    }    

    
}  // namespace

// ---------------------------------------

namespace IpsiusService
{    
    using Utils::HostInf;

    class NObjHwFinder::DirectDevList
    {
        //Utils::ManagedList<HostInf> m_list;
        std::list<HostInf> m_list;

    public:

        bool Add(const HostInf& addr) // return false if duplicate
        {
            if (Exist(addr)) return false;
            m_list.push_back(addr);
            return true;
        }

        bool Remove(const HostInf& addr) // return false if not exist
        {
            std::list<HostInf>::iterator i;
            i = std::find(m_list.begin(), m_list.end(), addr);
            if (i == m_list.end()) return false;
            m_list.erase(i);
            return true;
        }

        void SendRequests(BfBootCli::BroadcastReceiver& bcReceiver) const        
        {
            std::list<HostInf>::const_iterator i;
            for(i = m_list.begin(); i != m_list.end(); ++i)
            {                                
                bcReceiver.DirectRequest(*i);                
            }
        }

        bool Exist(const HostInf& addr) const
        {
            return std::find(m_list.begin(), m_list.end(), addr) != m_list.end();
        }

        bool Empty() const
        {
            return m_list.empty();
        }
    };

    // обеспечивает информацию об aliases (привязанных к платах) и
    // клиентах, которые ожидают готовности этих плат
    // фактически это уровень, паралелльный NObjHwBoardList; 
    // BoardInfo не интегрирован в NObjHwBoard, т.к. NObjHwBoard
    // обслуживает только "реальные" платы (т.е. платы, от которых приходил
    // broadcast), а прослойка BoardInfoList может существовать вне этих
    // "реальных" плат 
    class NObjHwFinder::BoardInfoList
    {        
        class BoardInfo : boost::noncopyable
        {            
            const int m_hwNum;
            Utils::StringList m_aliases;
            SafeRef<IAllocBoard> m_callback;
            QString m_callbackAlias;
            QString m_ownerName;
            int m_callbackCounter;

            // empty if board in non-ready state 
            scoped_ptr<BfBootCore::BroadcastMsg> m_msg;            

            void TryDoCallbackAlloc()
            {
                if (m_msg == 0) return;
                if (!CanDoCallback()) return;                
                m_callbackCounter++;
                m_callback->BoardAllocated(m_callbackAlias, *m_msg);
            }

            void TryDoCallbackUnalloc()
            {
                if (!CanDoCallback()) return;
                m_callbackCounter++;
                m_callback->BoardUnallocated(m_callbackAlias);
            }

            bool CanDoCallback() const
            {
                return
                    !m_callback.IsEmpty() && HasAlias(m_callbackAlias);
            }

            static QString ResolveOwnerName(IAllocBoard *p)
            {
                ESS_ASSERT(p != 0);

                Domain::NamedObject *pObj = dynamic_cast<NamedObject*>(p);

                return (pObj == 0) ? "<undefined>" : pObj->Name().Name();
            }

        public:            

            BoardInfo(int hwNum, QString alias = "") : m_hwNum(hwNum),
                m_callbackCounter(0)
            {                
                if (alias.isEmpty()) return;
                m_aliases.push_back(alias); 
            }           

            bool HasAlias(const QString &alias) const
            {
                return m_aliases.contains(alias);
            }            

            int HwNumber() const { return m_hwNum; }

            void AddAlias(const QString &alias)
            {
                ESS_ASSERT( !HasAlias(alias) );
                m_aliases.push_back(alias);
            }

            bool RemoveAlias(const QString &alias) 
            {
                int itemIndx = m_aliases.indexOf(alias);
                ESS_ASSERT(itemIndx >= 0);

                if (alias == m_callbackAlias) return false;

                m_aliases.removeAt(itemIndx);

                return true;
            }

            void GoReady(const BfBootCore::BroadcastMsg &msg)
            {
                m_msg.reset( new BfBootCore::BroadcastMsg(msg) );

                TryDoCallbackAlloc();                
            }

            bool Allocate(QString alias, SafeRef<IAllocBoard> callback)
            {
                if (!m_callback.IsEmpty()) return false;  // already used
                if (alias.isEmpty()) return false;        // bad alias

                m_callback = callback;
                m_callbackAlias = alias;
                m_ownerName = ResolveOwnerName(callback.operator->());
                m_callbackCounter = 0;
                 
                TryDoCallbackAlloc();      

                return true;
            }

            void GoNotReady()
            {                
                m_msg.reset();
                TryDoCallbackUnalloc();
            }

            bool Free(QString alias)
            {
                if (m_callback.IsEmpty()) return false;
                if (m_callbackAlias != alias) return false;

                m_callback.Clear();
                m_callbackAlias.clear();
                m_ownerName.clear();
                m_callbackCounter = 0;

                return true;
            }

            QString OwnerName() const { return m_ownerName; }

            QString Info() const
            {
                QString res;

                res += QString("Board %1").arg(m_hwNum);
                
                if (m_msg != 0) res += "+";
                
                // aliases
                for(int i = 0; i < m_aliases.size(); ++i)
                {
                    if (i == 0) res += "; ";

                    res += m_aliases.at(i);
                    if (m_aliases.at(i) == m_callbackAlias) res += "*";

                    if (i != m_aliases.size() - 1) res += ", ";
                }

                // owner + callback
                if (!m_callback.IsEmpty())
                {
                    res += QString("; %1 %2").arg(m_ownerName).arg(m_callbackCounter);
                }

                return res;
            }

        };

        Utils::ManagedList<BoardInfo> m_list;

        int Find(QString alias) const
        {
            for(int i = 0; i < m_list.Size(); ++i)
            {
                if (m_list[i]->HasAlias(alias)) return i;                
            }

            return -1;
        }

        BoardInfo* Find(int number) const
        {
            for(int i = 0; i < m_list.Size(); ++i)
            {
                if (m_list[i]->HwNumber() == number) return m_list[i];                
            }

            return 0;
        }
     
        
    public:

        BoardInfoList()
        {
        }    

        bool Alias(int hwNumber, QString alias)
        {
            if (alias.isEmpty()) return false;

            int indx = Find(alias);
            if (indx >= 0) return false;  // dublicate

            BoardInfo *pBoard = Find(hwNumber);

            if (pBoard == 0)  // add new info
            {
                m_list.Add( new BoardInfo(hwNumber, alias) );
            }
            else  // add alias
            {
                pBoard->AddAlias(alias);
            }

            return true;
        }
        

        bool Unalias(QString alias)
        {
            if (alias.isEmpty()) return false;

            int indx = Find(alias);
            if (indx < 0) return false;  // not found

            return m_list[indx]->RemoveAlias(alias);
        }        

        void Clear()
        {
            m_list.Clear();
        }

        void BoardEnterReadyState(const NObjHwBoard &board)
        {
            BoardInfo *pBoard = Find( board.HwNumber() );

            if (pBoard == 0)
            {
                m_list.Add( new BoardInfo(board.HwNumber()) );
                pBoard = m_list.Back();
            }

            pBoard->GoReady( board.Msg() );
        }

        void BoardLeaveReadyState(const NObjHwBoard &board)
        {
            BoardInfo *pBoard = Find( board.HwNumber() );            
            if (pBoard == 0) return;

            pBoard->GoNotReady();
        }

        bool AllocBoard(QString alias, SafeRef<IAllocBoard> callback)
        {
            int indx = Find(alias);
            if (indx < 0) return false;

            return m_list[indx]->Allocate(alias, callback);
        }

        bool FreeBoard(QString alias)
        {
            int indx = Find(alias);
            if (indx < 0) return false;

            return m_list[indx]->Free(alias);
        }

        QString FindOwner(const NObjHwBoard &board) const 
        {
            BoardInfo *pBoard = Find( board.HwNumber() );

            if (pBoard != 0) return pBoard->OwnerName();

            return "";
        }

        QStringList List() const
        {
            QStringList res;

            for(int i = 0; i < m_list.Size(); ++i)
            {
                res.push_back( m_list[i]->Info() );
            }

            return res;
        }
    };
    
}  // namespace IpsiusService


// ---------------------------------------

namespace IpsiusService
{

    NObjHwFinder::NObjHwFinder( IDomain *pDomain, const ObjectName &name ) : 
        NamedObject(pDomain, DriName()),
        m_totalBroadcastCounter(0),
        m_filteredBroadcastCounter(0),
        m_allowUpdate(true),
        m_onlyDirectDevice(false),
        m_onlyLocalIp(false),
        m_rollbackMode(false),
        m_pFilter(new NObjBoardFilter(this, "BoardFilter") ),
        m_pBoards(0),
        m_requestTimer(this, &NObjHwFinder::OnRequestTime)
    {
        m_enabled = false;
        m_firmwarePath = "../firmware";
        m_broadcastListenPort = BfBootCore::CBroadcastClientPort;       
		
		m_traceUpdateDetail = false;
        m_traceUpdate = false;
    }

    NObjHwFinder::~NObjHwFinder()
    {        
        SetEnabled(false);
    }

    // ------------------------------------------------------------
    
    void NObjHwFinder::SetEnabled( bool val )
    {
        if (val == m_enabled) return;

        if (val)  // try to goes enabled
        {
            m_totalBroadcastCounter = 0;
            m_filteredBroadcastCounter = 0;

            if (m_allowUpdate)
            {
                QString exeDir(Utils::ExeName::GetExeDir().c_str());

                QDir dir((QDir::isAbsolutePath(m_firmwarePath)) ? 
                m_firmwarePath :
                exeDir + m_firmwarePath);

                if (!dir.exists()) ThrowRuntimeException("FirmwarePath not exists");
                m_firmware.reset( new HwFirmwareStore(dir.path()) );
            }            

            m_broadcastRecv.reset( 
                new BfBootCli::BroadcastReceiver(getDomain().getMsgThread(), *this, m_broadcastListenPort) 
                );

            m_pBoards = new NObjHwBoardList(this, Name().AddChild("Boards"), *this); 

            m_aliases.reset( new BoardInfoList() );
            m_directDevs.reset( new DirectDevList() );
            m_pFilter->LockChanges(true);
        }
        else
        {
            m_pFilter->LockChanges(false);
            delete m_pBoards;
            m_broadcastRecv.reset();
            m_firmware.reset();
            m_aliases.reset();
            m_directDevs.reset();
            m_requestTimer.Stop();
        }

        m_enabled = val;
    }

    // ------------------------------------------------------

    void NObjHwFinder::BoardEnterReadyState(const NObjHwBoard &board)
    {
        if (m_aliases == 0) return;

        m_aliases->BoardEnterReadyState(board);
    }

    void NObjHwFinder::BoardLeaveReadyState(const NObjHwBoard &board)
    {
        if (m_aliases == 0) return;

        m_aliases->BoardLeaveReadyState(board);
    }


    // ------------------------------------------------------

    bool NObjHwFinder::AllocBoard( QString alias, SafeRef<IAllocBoard> callback )
    {
        if (m_aliases == 0) return false;

        return m_aliases->AllocBoard(alias, callback);
    }

    // ------------------------------------------------------

    bool NObjHwFinder::FreeBoard( QString alias )
    {
        if (m_aliases == 0) return false;

        return m_aliases->FreeBoard(alias);
    }

    // ------------------------------------------------------

    void NObjHwFinder::Alias( int hwNumber, QString alias )
    {
        ThrowIfNotEnabled();

        if (!m_aliases->Alias(hwNumber, alias))
        {
            ThrowRuntimeException("Alias already exists!");
        }
    }

    // ------------------------------------------------------

    void NObjHwFinder::Unalias( QString alias )
    {
        ThrowIfNotEnabled();

        if (!m_aliases->Unalias(alias))
        {
            ThrowRuntimeException("No such alias!");
        }        
    }

    // ------------------------------------------------------

    QString NObjHwFinder::GetBoardOwner(const NObjHwBoard &board) const
    {
        if (m_aliases == 0) return "";

        return m_aliases->FindOwner(board);
    }

    // ------------------------------------------------------

    bool NObjHwFinder::BoardTraceEnabled() const
    {
        return IsTagActive(InfoTag);
    }

    // ------------------------------------------------------

    void NObjHwFinder::BoardTrace( QString src, QString msg )
    {
		if (IsLogActive(InfoTag)) Log() << src << ": " << msg << iLogW::EndRecord;
    }

    // ------------------------------------------------------

    void NObjHwFinder::ResetBoard( const BfBootCore::BroadcastMsg &msg ) const
    {
        if (m_broadcastRecv == 0) return;       
        m_broadcastRecv->ResetDevice(msg);
    }

    // ------------------------------------------------------

    void NObjHwFinder::ListAliases( DRI::ICmdOutput *pCmd ) const
    {
        ThrowIfNotEnabled();

        ESS_ASSERT(m_aliases != 0);
        pCmd->Add( m_aliases->List() );
    }

    // ------------------------------------------------------

    void NObjHwFinder::ListBoards( DRI::ICmdOutput *pCmd, bool briefly /*= true*/ )
    {
        ThrowIfNotEnabled();
        m_pBoards->ListBoards(pCmd, briefly);
    }

    // ------------------------------------------------------

    void NObjHwFinder::MsgReceived( const BfBootCore::BroadcastMsg &msg )
    {   
        if ( m_onlyLocalIp && !Utils::IsLocalIp(msg.SrcAddress.Address().c_str()) ) return;        

        m_totalBroadcastCounter++;                    
        if (m_pBoards == 0) return;                
        if (msg.IsDirect() && !m_directDevs->Exist(msg.SrcAddress)) return;

        if ( m_onlyDirectDevice && !msg.IsDirect() ) return;
        
        m_pBoards->BroadcastMsg(msg);
    }

    // ------------------------------------------------------

    void NObjHwFinder::Clean()
    {
        ThrowIfNotEnabled();

        ESS_ASSERT(m_pBoards != 0);
        m_pBoards->Clean();
    }

    // ------------------------------------------------------

    void NObjHwFinder::ListFirmware( DRI::ICmdOutput *pCmd, bool withBadItems /*= false*/ )
    {
        ThrowIfNotEnabled();        

        if (!m_allowUpdate) return;

        ESS_ASSERT(m_firmware != 0);

        Utils::StringList sl;
        m_firmware->ListFirmware(sl, withBadItems);
        pCmd->Add(sl);
    }

    // ------------------------------------------------------

    /*

        def InsertEnvVarsInPath(path : str) -> str:
            
            CMarker = '%'
            
            res = path

            while True:
                posBegin = res.find(CMarker)
                if posBegin < 0: break
                posEnd = res.find(CMarker, posBegin + 1)
                assert posEnd >= 0, "Bad env in path " + path
                
                env = res[posBegin + 1:posEnd]
                envVal = os.getenv(env)
                assert envVal is not None, "Can't find env {0} in path {1}".format(env, path)
                res = res[:posBegin] + envVal + res[posEnd + 1:]
                                                    
            return res

    */

    void NObjHwFinder::Scan(IBootUpdater& updater, Platform::dword newVersion)
    {
        ThrowIfNotEnabled();
        m_pBoards->Scan(updater, newVersion);
    }

    int NObjHwFinder::getWrongMsgCount() const
    {
        if (!m_enabled) return 0;
        return m_broadcastRecv->WrongMsgCount();
    }        

    void NObjHwFinder::AddDirectBoard(QString boardAddr, int udpAppPort/* = 4096*/)
    {
        ThrowIfNotEnabled();        

        if ( !m_directDevs->Add(ResolveAddres(boardAddr, udpAppPort)) ) 
            ThrowRuntimeException("Duplicate address");        

        m_requestTimer.Start(BfBootCore::CBroadcastSendPeriod, true);
    }

    void NObjHwFinder::RemoveDirectBoard( QString boardAddr, int udpAppPort /*= 4096*/ )
    {
        ThrowIfNotEnabled();                

        HostInf addr = ResolveAddres(boardAddr, udpAppPort);
        
        if ( !m_directDevs->Remove(addr) ) ThrowRuntimeException("Address not exist");  

        m_pBoards->RemoveDirectBoards(addr);
        if (m_directDevs->Empty()) m_requestTimer.Stop();
    }

    void NObjHwFinder::OnRequestTime( const iCore::MsgTimer* )
    {
        if(!m_enabled) return;
        m_directDevs->SendRequests(*m_broadcastRecv);
    }

    void NObjHwFinder::ThrowIfEnabled() const
    {
        if (!m_enabled) return;
        ThrowRuntimeException("Object is enabled!");
    }

    void NObjHwFinder::ThrowIfNotEnabled() const
    {
        if (m_enabled) return;
        ThrowRuntimeException("Object is disabled!");
    }

    void NObjHwFinder::SetBroadcastListenPort( int val )
    {
        ThrowIfEnabled();
        m_broadcastListenPort = val;
    }

    void NObjHwFinder::SetFirmwarePath( QString val )
    {
        ThrowIfEnabled();
        m_firmwarePath = Utils::EnvVarsInPath::Insert(val);
    }

    bool NObjHwFinder::TaskMngTraceEnabled() const
    {
        return m_traceUpdateDetail;
    }

    shared_ptr<HwFirmware> NObjHwFinder::FirmwareRequest( const BfBootCore::BroadcastMsg &msg ) const
    {
        if (m_firmware == 0) return shared_ptr<HwFirmware>();

        return m_firmware->UpdateRequest(msg, m_rollbackMode);
    }

    bool NObjHwFinder::IsFiltered( const BfBootCore::BroadcastMsg &msg ) const
    {
        return m_pFilter->AcceptedBoard(msg);
    }

    bool NObjHwFinder::IsDirect( const BfBootCore::BroadcastMsg &msg ) const
    {
        return m_directDevs->Exist(msg.SrcAddress);
    }

    IpsiusService::HostInf NObjHwFinder::ResolveAddres(QString ip, int port)
    {        
        try
        {
            HostInf addr(ip.toStdString(), port);
            return addr;
        }
        catch(const HostInf::BadAddress& e)
        {
            ThrowRuntimeException(e.getTextMessage());
        }
        return HostInf();
    }

	bool NObjHwFinder::TraceProgress() const
	{
		return m_traceUpdate;
	}
    
}  // namespace IpsiusService

