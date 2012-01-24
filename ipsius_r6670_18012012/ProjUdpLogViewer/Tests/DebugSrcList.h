#ifndef __DEBUGSRCLIST__
#define __DEBUGSRCLIST__

#include "Core/UlvTypes.h"

#include "iNet/MsgUdpSocket.h"
#include "iNet/SocketError.h"
#include "Utils/ManagedList.h"
#include "Utils/Random.h"
#include "Utils/SafeRef.h"
#include "iCore/MsgTimer.h"
#include "iCore/MsgObject.h"



namespace UlvTests
{
    using namespace Ulv;

    // Call-back interface to debug user
    class IDebugErrorHandler : public Utils::IBasicInterface
    {
    public:
        virtual void DebugError(const QString &error) = 0;
    };

    // --------------------------------------------------------------------------

    // Send data via UDP port to the one of the application's open ports
    class DebugSource:
        public iCore::MsgObject,
        public iNet::IUdpSocketToUser,        
        boost::noncopyable
    {
        typedef DebugSource T;

        IDebugErrorHandler &m_errHandler;

        iNet::MsgUdpSocket m_socket;
        iCore::MsgTimer m_timer;        
        dword m_timerStartTimeMs;

        Utils::HostInf m_rcvr;
        UdpPortDataType m_dataType;
        word m_packsPerSec;
        QString m_name;
        QString m_dtFormat;
        dword m_sentPackCount;
        bool m_isRunning;
        Utils::Random m_random; // to generate random strings

        QByteArray MakeSendingString();
        QString RandomData();
        void StartTimer(bool resetStartTime = true);
        void StopTimer();
        void OnTimer(iCore::MsgTimer *pT);
        void OnReceiveData(boost::shared_ptr<iNet::SocketData> data);
        void OnSocketErrorOccur(boost::shared_ptr<iNet::SocketError> error);


    // iNet::IUdpSocketToUser impl
    private:
        void ReceiveData(boost::shared_ptr<iNet::SocketData> data);
        void SocketErrorOccur(boost::shared_ptr<iNet::SocketError> error);

    public:
        DebugSource(iCore::MsgThread &thread, IDebugErrorHandler &errHandler,
                    int rcvrPort, word packsPerSec,
                    UdpPortDataType dataType, const QString &name,
                    const QString &dateTimeFormat);
        ~DebugSource();

        void Run();
        void Stop();

        bool IsRunning() const;

        dword SentPackCount();
        QString Info();
    };

    // --------------------------------------------------------------------------

    class IDebugSrcListAsyncTasks : public Utils::IBasicInterface
    {
    public:
        virtual void AsyncDelete(int index) = 0;
        virtual void AsyncClear() = 0;
    };

    // --------------------------------------------------------------------------

    class DebugSrcListWidget :
        public QListWidget
    {
        Q_OBJECT

        iCore::MsgThread m_thread; // for sockets

        Utils::ManagedList<DebugSource> m_data;
        IDebugErrorHandler *m_pErrHandler;
        QString m_dtFormat;
        bool m_setupComplete;

        int m_counter; // debug #

        // debug context menu actions
        QAction *m_runStop; // checkable
        QAction *m_deleteSrc;
        QAction *m_deleteAllSrc;
        QAction *m_stopAllSrc;
        QAction *m_runAllSrc;

        void ConnectSigSlots();

        void Run(int index);
        void Stop(int index);
        void Delete(int index);
        void Refresh(int index);
        bool IsRunning(int index) const;

    private slots:
        void OnCustomContextMenuRequested(QPoint pos);

        // for debug context menu
        void OnRunStopSrc();
        void OnDeleteSrc();
        void OnDeleteAllSrc();
        void OnStopAllSrc();
        void OnRunAllSrc();

    public:
        DebugSrcListWidget(QWidget *pParent);

        void Setup(IDebugErrorHandler *pErrHandler, const QString &dtFormat);

        void AddAndRun(int rcvrPort, word packsPerSec, UdpPortDataType dataType);

        void Refresh();
        void Clear();

        int ActiveCount() const;
        int Count() const;
    };

    // --------------------------------------------------------------------------

    // Selt-test mechanism
    /*class DebugSrcList :
        public virtual Utils::SafeRefServer,
        public IDebugSrcListAsyncTasks
    {
        IDebugErrorHandler &m_errHandler;
        QString m_dtFormat;

        iCore::MsgThread m_thread; // for sockets
        Utils::ManagedList<DebugSource> m_list;

        int m_counter; // debug #

    // IDebugSrcListAsyncTasks impl
    private:
        void AsyncDelete(int index);
        void AsyncClear();

    public:
        DebugSrcList(IDebugErrorHandler &errHandler, const QString &dateTimeFormat);

        // Returns generated source info
        QString Add(int rcvrPort, word packsPerSec, UdpPortDataType dataType);

        DebugSource& operator[](size_t index);
        const DebugSource& operator[](size_t index) const;

        size_t Size() const { return m_list.Size(); }

        void Delete(size_t index);
        void Clear();
    };*/

} // namespace UlvTests

#endif
