
#include "stdafx.h"
#include "Tests/DebugSrcList.h"

#include "Utils/BinaryWriter.h"
#include "Utils/BinaryReader.h"
#include "Utils/MemWriterStream.h"
#include "Utils/MemReaderStream.h"
#include "Utils/TimerTicks.h" // for random seed
#include "iCore/SyncTimer.h"


namespace
{
    using namespace UlvTests;

    const dword CMinTimerIntervalMs = 100; // for >= 10 packs/sec

    const int CInvalidPackNumberRange = 100;

    const QString CSentData = "A QDateTime object |\n| contains |\n|a calendar date and a clock time |\r\n|"
                              "(a \"\tdatetime\t\"). |\n|It is a combination of the QDate and |\n|"
                              "QTime classes. |\n|It can read the current datetime |\r\n|"
                              "from the system |\n| clock.";


    // CInvalidPacket.size() % sizeof(wchar_t) has to be 0!
    const QByteArray CInvalidPacket = "[some] inva\nlid [data] [[[[][";

} // namespace

// -----------------------------------------------------------------------

/*namespace
{
    class AsyncClearTask : public iCore::MsgThread::AsyncTask
    {
        Utils::SafeRef<IDebugSrcListAsyncTasks> m_task;

    public:
        AsyncClearTask(Utils::SafeRef<IDebugSrcListAsyncTasks> task) :
            m_task(task)
        {
        }

        void Execute() // override
        {
            m_task->AsyncClear();
        }
    };

    // -----------------------------------------------------------------------

    class AsyncDeleteTask : public iCore::MsgThread::AsyncTask
    {
        Utils::SafeRef<IDebugSrcListAsyncTasks> m_task;
        int m_index;

    public:
        AsyncDeleteTask(Utils::SafeRef<IDebugSrcListAsyncTasks> task, int index) :
            m_task(task), m_index(index)
        {
        }

        void Execute() // override
        {
            m_task->AsyncDelete(m_index);
        }
    };
}*/

// -----------------------------------------------------------------------

namespace UlvTests
{
    // -----------------------------------------------------------------------
    // DebugSource impl

    DebugSource::DebugSource(iCore::MsgThread &thread, IDebugErrorHandler &errHandler,
                             int rcvrPort, word packsPerSec, UdpPortDataType dataType,
                             const QString &name, const QString &dateTimeFormat) :
        iCore::MsgObject(thread), m_errHandler(errHandler),
        m_socket(thread, this, Utils::HostInf(0)),
        m_timer(this, &T::OnTimer), m_timerStartTimeMs(0),
        m_rcvr(Utils::HostInf("127.0.0.1", rcvrPort)),
        m_dataType(dataType), m_packsPerSec(packsPerSec), m_name(name),
        m_dtFormat(dateTimeFormat),
        m_sentPackCount(0), m_isRunning(false),
        m_random(Utils::TimerTicks::Ticks())
    {
    }

    // -----------------------------------------------------------------------

    DebugSource::~DebugSource()
    {
        StopTimer();
    }

    // -----------------------------------------------------------------------

    void DebugSource::OnTimer(iCore::MsgTimer *pT)
    {
        // ignore messages left after timer stopped
        if (!m_isRunning) return;
        
        StopTimer();

        dword timeElapsedMs = iCore::SyncTimer::Ticks() - m_timerStartTimeMs;

        dword needToSendPacks =  m_packsPerSec * timeElapsedMs / 1000;
        if (needToSendPacks == 0)
        {
            // don't reset start time and start timer again
            StartTimer(false);
            return;
        }

        int invalidPackNum = m_random.Next(CInvalidPackNumberRange) + 1;

        // send
        dword lastPack = m_sentPackCount + needToSendPacks;
        while (m_sentPackCount < lastPack)
        {
            // send each 10th pack invalid
            if ((m_sentPackCount % invalidPackNum) == 0)
            {
                m_socket.SendData(boost::shared_ptr<iNet::SocketData>(
                                    new iNet::SocketData(m_rcvr, CInvalidPacket)));
            }
            else
            {
                m_socket.SendData(boost::shared_ptr<iNet::SocketData>(
                                    new iNet::SocketData(m_rcvr, MakeSendingString())));
            }
            ++m_sentPackCount;
        }

        // restart timer
        StartTimer();
    }

    // -----------------------------------------------------------------------

    void DebugSource::StartTimer(bool resetStartTime)
    {
        if (resetStartTime) m_timerStartTimeMs = iCore::SyncTimer::Ticks();
        m_timer.Start(CMinTimerIntervalMs);
        // m_isRunning = true;
    }

    // -----------------------------------------------------------------------

    void DebugSource::StopTimer()
    {
        m_timer.Stop();        
        // m_isRunning = false;
    }

    // -----------------------------------------------------------------------

    void DebugSource::Run()
    {
        StartTimer();
        m_isRunning = true;
    }

    // -----------------------------------------------------------------------

    void DebugSource::Stop()
    {
        StopTimer();
        m_isRunning = false;
    }

    // -----------------------------------------------------------------------

    bool DebugSource::IsRunning() const
    {
        return m_isRunning;
    }

    // -----------------------------------------------------------------------

    dword DebugSource::SentPackCount()
    {
        return m_sentPackCount;
    }

    // -----------------------------------------------------------------------

    QString DebugSource::Info()
    {
        QString res(m_name);
        res += " [sending to localhost:";
        res += QString::number(m_rcvr.Port());
        res += ", ";
        res += QString::number(m_packsPerSec);
        res += " packets/sec, ";
        res += (m_dataType == PTString)? "ASCII" : "UNICODE";
        res += " data]: ";
        res += QString::number(SentPackCount());

        return res;
    }

    // -----------------------------------------------------------------------

    void DebugSource::ReceiveData(boost::shared_ptr<iNet::SocketData> data)
    {
        PutMsg(this, &T::OnReceiveData, data);
    }

    // -----------------------------------------------------------------------

    void DebugSource::SocketErrorOccur(boost::shared_ptr<iNet::SocketError> error)
    {
        PutMsg(this, &T::OnSocketErrorOccur, error);
    }

    // -----------------------------------------------------------------------

    void DebugSource::OnReceiveData(boost::shared_ptr<iNet::SocketData> data)
    {
        QString err("Debug error: sender received data.");
        m_errHandler.DebugError(err);
    }

    // -----------------------------------------------------------------------

    void DebugSource::OnSocketErrorOccur(boost::shared_ptr<iNet::SocketError> error)
    {
        QString err("Debug error: ");
        err += error->getErrorString();
        m_errHandler.DebugError(err);
    }

    // -----------------------------------------------------------------------

    QByteArray DebugSource::MakeSendingString()
    {
        // make string

        QString data;
        // data += "[";
        data += QDateTime::currentDateTime().toString(m_dtFormat);
        // data += "]";
        data += "[";
        data += m_name;
        data += "][tag]";
        // data += "[";
        data += "(";
        data += QString::number(m_sentPackCount);
        data += ")";
        // get random part of string
        data += RandomData();
        // data += "]";

        // debug
        data += "\t(dataEnd)";

        // return as array of bytes
        QByteArray res;
        switch (m_dataType)
        {
        case Ulv::PTString:
            res.append(data);
            break;

        case Ulv::PTWstring:
            {
                int step = sizeof(wchar_t);

                for (size_t i = 0; i < data.size(); ++i)
                {
                    wchar_t wChar = data.at(i).unicode();

                    for (int chI = (step - 1); chI >= 0; --chI)
                    {
                        res += ((wChar >> (chI * 8)) & 0xff);
                    }
                }
            }
            break;

        default:
            ESS_HALT("Invalid data type");
        }

        return res;
    }

    // -----------------------------------------------------------------------

    QString DebugSource::RandomData()
    {
        dword size = CSentData.size();

        dword startPos = m_random.Next(size);
        dword count = m_random.Next(size);

        if ((startPos + count) > size) count = size - startPos;

        return CSentData.mid(startPos, count);
    }

    // -----------------------------------------------------------------------
    // DebugSrcListWidget impl

    DebugSrcListWidget::DebugSrcListWidget(QWidget *pParent) :
        QListWidget(pParent),
			m_thread("DebugSrcListWidget::m_thread", Platform::Thread::LowPriority, true),
        m_data(true),
        m_pErrHandler(0),
        m_setupComplete(false),
        m_counter(0)
    {
        ConnectSigSlots();
    }

    // -----------------------------------------------------------------------

    void DebugSrcListWidget::Setup(IDebugErrorHandler *pErrHandler, const QString &dtFormat)
    {
        ESS_ASSERT(pErrHandler != 0);
        m_pErrHandler = pErrHandler;
        m_dtFormat = dtFormat;
        m_setupComplete = true;
    }

    // -----------------------------------------------------------------------

    void DebugSrcListWidget::ConnectSigSlots()
    {
        connect(this, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(OnCustomContextMenuRequested(QPoint)));

        // actions
        m_runStop = new QAction("Running", this);
        m_runStop->setCheckable(true);
        m_runStop->setChecked(false);
        connect(m_runStop, SIGNAL(triggered()), this, SLOT(OnRunStopSrc()));

        m_deleteSrc = new QAction("Delete", this);
        connect(m_deleteSrc, SIGNAL(triggered()), this, SLOT(OnDeleteSrc()));

        m_deleteAllSrc = new QAction("Delete all", this);
        connect(m_deleteAllSrc, SIGNAL(triggered()), this, SLOT(OnDeleteAllSrc()));

        m_stopAllSrc = new QAction("Stop all", this);
        connect(m_stopAllSrc, SIGNAL(triggered()), this, SLOT(OnStopAllSrc()));

        m_runAllSrc = new QAction("Run all", this);
        connect(m_runAllSrc, SIGNAL(triggered()), this, SLOT(OnRunAllSrc()));
    }

    // -----------------------------------------------------------------------

    void DebugSrcListWidget::Run(int index)
    {
        ESS_ASSERT(index < Count());

        m_data[index]->Run();
        Refresh(index);

    }

    // -----------------------------------------------------------------------

    void DebugSrcListWidget::Stop(int index)
    {
        ESS_ASSERT(index < Count());

        m_data[index]->Stop();
        Refresh(index);
    }

    // -----------------------------------------------------------------------

    void DebugSrcListWidget::Delete(int index)
    {
        ESS_ASSERT(index < Count());

        m_data.Delete(index);
        delete this->takeItem(index);
    }

    // -----------------------------------------------------------------------

    void DebugSrcListWidget::Refresh(int index)
    {
        ESS_ASSERT(index < Count());

        this->item(index)->setText(m_data[index]->Info());
    }

    // -----------------------------------------------------------------------

    bool DebugSrcListWidget::IsRunning(int index) const
    {
        ESS_ASSERT(index < Count());

        return m_data[index]->IsRunning();
    }

    // -----------------------------------------------------------------------

    void DebugSrcListWidget::OnCustomContextMenuRequested(QPoint pos)
    {
        if (Count() == 0) return;

        // modify action for current item
        m_runStop->setChecked(IsRunning(this->currentRow()));

        // make menu
        QMenu menu(this);
        menu.addAction(m_runStop);
        menu.addAction(m_deleteSrc);
        menu.addSeparator();
        menu.addAction(m_runAllSrc);
        menu.addAction(m_stopAllSrc);
        menu.addSeparator();
        menu.addAction(m_deleteAllSrc);

        menu.exec(this->cursor().pos());
    }

    // -----------------------------------------------------------------------

    void DebugSrcListWidget::OnRunStopSrc()
    {
        if (Count() == 0) return;

        int index = this->currentRow();

        // stop
        if (IsRunning(index))
        {
            Stop(index);
            return;
        }

        // run
        Run(index);
    }

    // -----------------------------------------------------------------------

    void DebugSrcListWidget::OnDeleteSrc()
    {
        if (Count() == 0) return;

        Delete(this->currentRow());
    }

    // -----------------------------------------------------------------------

    void DebugSrcListWidget::OnDeleteAllSrc()
    {
        for (int i = Count() - 1; i >= 0; --i)
        {
            Delete(i);
        }
    }

    // -----------------------------------------------------------------------

    void DebugSrcListWidget::OnStopAllSrc()
    {
        if (Count() == 0) return;

        for (int i = 0; i < Count(); ++i)
        {
            if (IsRunning(i)) Stop(i);
        }
    }

    // -----------------------------------------------------------------------

    void DebugSrcListWidget::OnRunAllSrc()
    {
        if (Count() == 0) return;

        for (int i = 0; i < Count(); ++i)
        {
            if (!IsRunning(i)) Run(i);
        }
    }

    // -----------------------------------------------------------------------

    void DebugSrcListWidget::Refresh()
    {
        for (int i = 0; i < Count(); ++i)
        {
            Refresh(i);
        }
    }

    // -----------------------------------------------------------------------

    void DebugSrcListWidget::AddAndRun(int rcvrPort, word packsPerSec, UdpPortDataType dataType)
    {
        ESS_ASSERT(m_setupComplete);

        QString name = QString("Debug %1").arg(m_counter);
        ++m_counter;

        DebugSource *pData = new DebugSource(m_thread, *m_pErrHandler, rcvrPort, packsPerSec,
                                             dataType, name, m_dtFormat);

        m_data.Add(pData); // handle deletion

        //QListWidgetItem *pItem = new QListWidgetItem(pData->Info());
        //pItem->setData(Qt::UserRole, pData);

        // add to GUI
        this->addItem(pData->Info());

        // run
        pData->Run();
    }

    // -----------------------------------------------------------------------

    void DebugSrcListWidget::Clear()
    {
        OnDeleteAllSrc();
    }

    // -----------------------------------------------------------------------

    int DebugSrcListWidget::ActiveCount() const
    {
        int res = 0;
        for (int i = 0; i < Count(); ++i)
        {
            if (IsRunning(i)) ++res;
        }

        return res;
    }

    // -----------------------------------------------------------------------

    int DebugSrcListWidget::Count() const
    {
        ESS_ASSERT(m_data.Size() == this->count());
        return this->count();
    }

    // -----------------------------------------------------------------------



    // -----------------------------------------------------------------------
    /*// DebugSrcList impl

    DebugSrcList::DebugSrcList(IDebugErrorHandler &errHandler, const QString &dateTimeFormat):
        m_errHandler(errHandler), m_dtFormat(dateTimeFormat),
        m_thread(Platform::Thread::LowPriority, true), m_counter(0)
    {
    }

    // -----------------------------------------------------------------------

    QString DebugSrcList::Add(int rcvrPort, word packsPerSec, UdpPortDataType dataType)
    {
        QString name = QString("Debug %1").arg(m_counter);

        m_list.Add(new DebugSource(m_thread, m_errHandler, rcvrPort, packsPerSec,
                                   dataType, name, m_dtFormat));

        ++m_counter;

        return m_list[Size() - 1]->Info();
    }

    // -----------------------------------------------------------------------

    DebugSource& DebugSrcList::operator[](size_t index)
    {
        ESS_ASSERT(index < Size());

        return *m_list[index];
    }

    // -----------------------------------------------------------------------

    const DebugSource& DebugSrcList::operator[](size_t index) const
    {
        ESS_ASSERT(index < Size());

        return *m_list[index];
    }

    // -----------------------------------------------------------------------

    void DebugSrcList::Clear()
    {
        boost::shared_ptr<AsyncClearTask> task(new AsyncClearTask(this));
        m_thread.RunAsyncTask(task);
    }

    // -----------------------------------------------------------------------

    void DebugSrcList::Delete(size_t index)
    {
        ESS_ASSERT(index < Size());

        boost::shared_ptr<AsyncDeleteTask> task(new AsyncDeleteTask(this, index));
        m_thread.RunAsyncTask(task);
    }

    // -----------------------------------------------------------------------

    void DebugSrcList::AsyncDelete(int index)
    {
        m_list.Delete(index);
    }

    // -----------------------------------------------------------------------

    void DebugSrcList::AsyncClear()
    {
        m_counter = 0;

        m_list.Clear();
    }*/


} // namespace UlvTests
