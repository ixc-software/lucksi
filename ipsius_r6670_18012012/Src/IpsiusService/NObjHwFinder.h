#ifndef __NOBJHWFINDER__
#define __NOBJHWFINDER__

#include "Utils/SafeRef.h"
#include "Utils/ExeName.h"
#include "BfBootCli/BroadcastReceiver.h"
#include "BfBootCore/GeneralBooterConst.h"
#include "BfBootCore/BroadcastSourcePort.h"

#include "NObjHwBoardList.h"
#include "HwFirmwareStore.h"
#include "IAllocBoard.h"
#include "IBootUpdater.h"

namespace iLogW
{
    class LogSession;
}

namespace IpsiusService
{
    using Utils::SafeRef;
    using boost::scoped_ptr;

    class NObjBoardFilter;        

    // -------------------------------------------------------
    
    /*  Объект отвечают за обнаружение плат в сети, обновление прошивки на них,
        выделение этих плат в пользование другим объектам
    */
    class NObjHwFinder : 
        public NamedObject,
        BfBootCli::IBroadcastReceiverToOwner,
        IHwBoardOwner
    {
        Q_OBJECT;

        class BoardInfoList;
        class DirectDevList; 

        scoped_ptr<iLogW::LogSession> m_log;
		bool m_traceUpdateDetail;
        bool m_traceUpdate;
        iLogW::LogRecordTag m_tagInfo;

        bool m_enabled;
        QString m_firmwarePath;
        int m_broadcastListenPort;

        int m_totalBroadcastCounter;
        int m_filteredBroadcastCounter; 
        bool m_allowUpdate;
        bool m_onlyDirectDevice;
        bool m_rollbackMode;
        bool m_onlyLocalIp;

        NObjBoardFilter*const m_pFilter;

        // exists only in enabled state
        NObjHwBoardList *m_pBoards;  // it's child
        scoped_ptr<HwFirmwareStore> m_firmware;
        scoped_ptr<BfBootCli::BroadcastReceiver> m_broadcastRecv;
        scoped_ptr<BoardInfoList> m_aliases;

        scoped_ptr<DirectDevList> m_directDevs;

        iCore::MsgTimer m_requestTimer; // started if enabled                

        void OnRequestTime(const iCore::MsgTimer*);
        void SetFirmwarePath(QString val);
        void SetBroadcastListenPort(int val);
        void ThrowIfNotEnabled() const;
        void ThrowIfEnabled() const;
        int getWrongMsgCount() const;       
        Utils::HostInf ResolveAddres(QString ip, int port);


    // BfBootCli::IBroadcastReceiverToOwner impl
    private:

        void MsgReceived(const BfBootCore::BroadcastMsg &msg);

    // IHwBoardOwner impl
    private:
        shared_ptr<HwFirmware> FirmwareRequest(const BfBootCore::BroadcastMsg &msg) const;
        
        void BoardEnterReadyState(const NObjHwBoard &board);
        void BoardLeaveReadyState(const NObjHwBoard &board);

        QString GetBoardOwner(const NObjHwBoard &board) const;
        bool BoardTraceEnabled() const;
        void BoardTrace(QString src, QString msg);
        bool TaskMngTraceEnabled() const;
		bool TraceProgress() const;
        void ResetBoard(const BfBootCore::BroadcastMsg &msg) const;        
        bool IsFiltered(const BfBootCore::BroadcastMsg &msg) const;
        bool IsDirect(const BfBootCore::BroadcastMsg &msg) const;

    public:

        // объект должен быть только один, parent -> только ROOT, и только специальное имя
        NObjHwFinder(IDomain *pDomain, const ObjectName &name);

        ~NObjHwFinder();

        // callback *can* be called inside with function if board is ready 
        bool AllocBoard(QString alias, SafeRef<IAllocBoard> callback);
        bool FreeBoard(QString alias);        

        void Scan(IBootUpdater& updater, Platform::dword newVersion);
        void AllowUpdate(bool allow)
        {
            ThrowIfEnabled();
            m_allowUpdate = allow;
        }
        void SetEnabled(bool val);
        void SetOnlyDirectDevice(bool val)
        {
            ThrowIfEnabled();
            m_onlyDirectDevice = val;
        }

    // DRI interface 
    public:

        static QString DriName()
        {
            return "HwFinder";
        }

        Q_PROPERTY(bool Enabled READ m_enabled WRITE SetEnabled);
        Q_PROPERTY(QString FirmwarePath READ m_firmwarePath WRITE SetFirmwarePath);
        Q_PROPERTY(bool AllowUpdate READ m_allowUpdate WRITE AllowUpdate);
        Q_PROPERTY(int BroadcastListenPort READ m_broadcastListenPort WRITE SetBroadcastListenPort);
        Q_PROPERTY(bool TraceUpdateProcedure READ m_traceUpdate WRITE m_traceUpdate);
        Q_PROPERTY( int ToalBroadcastCounter READ m_totalBroadcastCounter );
        Q_PROPERTY( int FilteredBroadcastCounter READ m_filteredBroadcastCounter );        
        Q_PROPERTY(int WrongMsgCount READ getWrongMsgCount);
        Q_PROPERTY(bool OnlyLocalIP READ m_onlyLocalIp WRITE m_onlyLocalIp); // debug, listen only localIp dev
        Q_PROPERTY(bool OnlyDirectDev READ m_onlyDirectDevice WRITE SetOnlyDirectDevice);// debug        
		Q_PROPERTY(bool TraceUpdateProgress READ m_traceUpdate WRITE m_traceUpdate);

        // update fwu allways if not equal version
        Q_PROPERTY(bool RollbackMode READ m_rollbackMode WRITE m_rollbackMode); 

        Q_INVOKABLE void ListFirmware(DRI::ICmdOutput *pCmd, bool withBadItems = false);

        Q_INVOKABLE void Alias(int hwNumber, QString alias);
        Q_INVOKABLE void Unalias(QString alias);
        Q_INVOKABLE void ListAliases(DRI::ICmdOutput *pCmd) const;
        Q_INVOKABLE void Clean();

        Q_INVOKABLE void ListBoards(DRI::ICmdOutput *pCmd, bool briefly = true);                

        Q_INVOKABLE void AddDirectBoard(QString boardAddr, int udpAppPort = BfBootCore::CBroadcastSourcePort);
        Q_INVOKABLE void RemoveDirectBoard(QString boardAddr, int udpAppPort = BfBootCore::CBroadcastSourcePort);
        

    };
    
    
}  // namespace IpsiusService

#endif
