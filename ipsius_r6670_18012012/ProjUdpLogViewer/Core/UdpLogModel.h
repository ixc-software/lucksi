
#ifndef __UDPLOGMODEL__
#define __UDPLOGMODEL__

// UdpLogModel.h

#include "iCore/MsgThread.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"
#include "Utils/AtomicTypes.h"
#include "Utils/ThreadSyncEvent.h"
#include "Utils/StatisticElement.h"

#include "UdpMonitor.h"
#include "UlvStartupParams.h"
#include "UdpLogDB.h"
#include "IUdpLogModelToView.h"
#include "IUdpLogControllerToModel.h"
#include "CtrlToModelParams.h"
#include "ModelToViewParams.h"


namespace Ulv
{
    // Core part of UDP log viewer, works in its own thread
    class UdpLogModel :
        public iCore::MsgObject,
        public IUdpMonitorEvents,
        public IControllerToModel,
        boost::noncopyable
    {
        typedef UdpLogModel T;

        enum State
        {
            StInit = 0,
            StActive = 1,
            StExit = 2,
        };

        State m_state;
        StartupParams m_params;
        UdpLogDB m_db;
        UdpMonitor m_udpMonitor;

        boost::scoped_ptr<IUdpLogViewRender> m_viewRender;
        Utils::SafeRef<IModelToView> m_view;
        bool m_dbDropped;

        Utils::AtomicInt m_refreshMsgCount;
        std::queue<boost::shared_ptr<CtrlToModelParams> > m_refreshParams;

        Utils::AtomicBool &m_exit;

        Utils::StatisticElement<ddword, ddword> m_renderTime;
        int m_renderCounter;

        int m_debugLevel;

    // IUdpMonitorEvents impl
    private:
        void NewData(const QString &data, const Utils::HostInf &src);
        void ErrorOccur(boost::shared_ptr<Error> error);

    // IControllerToModel impl
    private:
        // void MsgRefreshView();
        void MsgRefreshView(boost::shared_ptr<CtrlToModelParams> params);
        void MsgPauseView(bool state);
        void MsgShutdown();

        void MsgCopyAllLogToClipboard();
        void MsgCopyDisplayedLogToClipboard(int startRecord, int count);
        void MsgSaveLogToFile(QString fileName);

        void MsgClearDB();

    // events
    private:
        void OnPauseView(bool state);
        void OnShutdown();

        struct CopyParams
        {
            // boost::shared_ptr<Utils::ThreadSyncEvent> Event;
            int StartRecord;
            int Count;

            CopyParams(int startRecord = 0, int count = -1) :
                /*Event(new Utils::ThreadSyncEvent()), */StartRecord(startRecord), Count(count)
            {}
        };

        void OnCopyLogToClipboard(CopyParams params);
        void OnSaveLogToFile(QString fileName);

        void OnEnableRecordSrc(QString name);
        void OnDisableRecordSrc(QString name);
        void OnChangeAllRecordSrcState(bool state);
        void OnClearDB();        
        void OnRefreshView(/*boost::shared_ptr<CtrlToModelParams> params*/);

    private:
        void DebugLvl1(const QString &text);
        void DebugLvl2(const QString &text);

    public:
        // manage pViewRender deletion
        UdpLogModel(iCore::MsgThread &thread,
                    const StartupParams &params,
                    Utils::SafeRef<IModelToView> view,
                    IUdpLogViewRender *pViewRender,
                    Utils::AtomicBool &exit);
        ~UdpLogModel();
    };


} // namespace Ulv

#endif
