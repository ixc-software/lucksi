
#include "stdafx.h"

#include "UdpLogViewer.h"

#include "Core/UdpLogDBStatus.h"
#include "Core/UdpLogModel.h"
#include "QtDeclaredMetaTypes.h"
#include "Core/UlvUtils.h"
#include "UlvGuiLogSrcWidget.h"


namespace
{
    const Ulv::dword CRefreshIntervalMs = 500;
    const Ulv::dword CSlowRefreshIntervalMs = 3 * 1000;

    // ---------------------------------------------------------

    const QString CLogDir = "Log/";
    const QString CLogFilePref = "Log";
    const QString CLogFileExt = "txt";

    // ---------------------------------------------------------


} // namespace

// --------------------------------------------------------------------------------------

namespace UlvGui
{

    UdpLogViewer::UdpLogViewer(IUdpLogViewerGui &owner,
                               const Ulv::StartupParams &params,
                               Utils::AtomicBool &modelClosed) :
        m_guiThreadId((int)QThread::currentThreadId()),
        m_owner(owner),
        m_modelThread("UdpLogViewer::ModelThread", Platform::Thread::LowPriority, true),
        m_model(new Ulv::UdpLogModel(m_modelThread, params, this, new GuiLogViewRender, modelClosed)),
        m_refreshTimer(this),
        m_currTimerIntervalMs(CRefreshIntervalMs),
        m_lastRefreshMcs(Platform::GetSystemMicrosecTickCount()),
        m_modelThreadIdSet(false)
    {
        ConnectSigSlots();

        // init GUI and send default CtrlToModelParams
        // m_gui.reset(new UdpLogViewerGUI(*this, params, buildInfo));
        m_model->MsgRefreshView(m_owner.CurrentParams());

        // start refresh timer
        m_refreshTimer.start(m_currTimerIntervalMs);
    }

    // --------------------------------------------------------------------------------------

    UdpLogViewer::~UdpLogViewer()
    {
        ESS_ASSERT(InGuiThread());
        m_refreshTimer.stop();
    }

    // --------------------------------------------------------------------------------------

    void UdpLogViewer::Update(boost::shared_ptr<Ulv::ModelToViewParams> params)
    {
        /*
            inside Model thread
        */
        ESS_ASSERT(!InGuiThread());
        ESS_ASSERT(params != 0);

        // paint log, update scroll and add/update src list
        emit GuiUpdate(params);
    }

    // --------------------------------------------------------------------------------------

    void UdpLogViewer::UpdateStatus(Ulv::UdpLogDBStatus status)
    {
        /*
            inside Model thread
        */
        ESS_ASSERT(!InGuiThread());

        if (!m_modelThreadIdSet)
        {
             int modelThreadId = (int)QThread::currentThreadId();
             m_modelThreadIdSet.Set(true);
             emit HasModelThreadId(modelThreadId);
        }

        emit GuiUpdateStatus(status);
    }

    // --------------------------------------------------------------------------------------

    void UdpLogViewer::DisplayError(Ulv::Error error)
    {
        /*
            inside Model or GUI (exceptions from constructors) thread
        */

        bool criticalErr = ((error.Type() == Ulv::Error::TCriticalErr)
                            || (error.Type() == Ulv::Error::TModelErr));

        if (InGuiThread())
        {
            OnGuiShowError(error.AsQString(), criticalErr);
            return;
        }

        emit GuiShowError(error.AsQString(), criticalErr);
    }

    // --------------------------------------------------------------------------------------

    void UdpLogViewer::SaveToClipboard(QString data)
    {
        /*
            inside Model thread
        */

        ESS_ASSERT(!InGuiThread());

        emit GuiSaveToClipboard(data);
    }

    // --------------------------------------------------------------------------------------
    // other

    bool UdpLogViewer::InGuiThread() const
    {
        return (m_guiThreadId == (int)QThread::currentThreadId());
    }

    // --------------------------------------------------------------------------------------

    void UdpLogViewer::ConnectSigSlots()
    {
        int id = qRegisterMetaType<Ulv::UdpLogDBStatus>();
        int id2 = qRegisterMetaType<boost::shared_ptr<Ulv::ModelToViewParams> >();

        connect(this, SIGNAL(GuiUpdate(boost::shared_ptr<Ulv::ModelToViewParams>)),
                this, SLOT(OnGuiUpdate(boost::shared_ptr<Ulv::ModelToViewParams>)));

        connect(this, SIGNAL(GuiUpdateStatus(Ulv::UdpLogDBStatus)),
                this, SLOT(OnGuiUpdateStatus(Ulv::UdpLogDBStatus)));

        connect(this, SIGNAL(GuiShowError(QString, bool)),
                this, SLOT(OnGuiShowError(QString, bool)));

        connect(this, SIGNAL(GuiSaveToClipboard(QString)),
                this, SLOT(OnGuiSaveToClipboard(QString)));

        connect(&m_refreshTimer, SIGNAL(timeout()), this, SLOT(OnTimerRefresh()));

        connect(this, SIGNAL(HasModelThreadId(int)), this, SLOT(OnHasModelThreadId(int)));

    }

    // --------------------------------------------------------------------------------------

    void UdpLogViewer::OnTimerRefresh()
    {
        m_lastRefreshMcs = Platform::GetSystemMicrosecTickCount();

        ESS_ASSERT(InGuiThread());

        if (m_owner.IsMinimized())
        {
            m_refreshTimer.start(m_currTimerIntervalMs);
            return;
        }

        m_model->MsgRefreshView(m_owner.CurrentParams());
        m_owner.UpdateDebug();

        if ((m_currTimerIntervalMs == CRefreshIntervalMs) && m_owner.IsNotInFocus())
        {
            m_currTimerIntervalMs = CSlowRefreshIntervalMs;
        }

        if ((m_currTimerIntervalMs == CSlowRefreshIntervalMs) && (!m_owner.IsNotInFocus()))
        {
            m_currTimerIntervalMs = CRefreshIntervalMs;
        }

        m_refreshTimer.start(m_currTimerIntervalMs);
    }

    // --------------------------------------------------------------------------------------

    void UdpLogViewer::OnHasModelThreadId(int id)
    {
        ESS_ASSERT(InGuiThread());
        m_owner.ShowInfo(QString("Model thread ID: %1").arg(id));
        m_owner.ShowInfo(QString("GUI thread ID: %1").arg(m_guiThreadId));
    }

    // --------------------------------------------------------------------------------------
    // Ulv::IModelToController impl

    /*void UdpLogViewer::MsgModelDisactivated()
    {
        emit GuiClose();
    }*/

    // --------------------------------------------------------------------------------------
    // public

    void UdpLogViewer::SaveToFile()
    {
        QString err;
        QString file = UlvUtils::GenAbsFileName(CLogDir, CLogFilePref, CLogFileExt, err);
        if (!err.isEmpty())
        {
            m_owner.ShowError(err, false);
            return;
        }

        m_model->MsgSaveLogToFile(file);

        // and note to info
        QString text = QString("Log was saved to file '%1'").arg(file);
        m_owner.ShowInfo(text);
    }

    // --------------------------------------------------------------------------------------

    void UdpLogViewer::Copy(int startRecord, int count)
    {
        m_model->MsgCopyDisplayedLogToClipboard(startRecord, count);
    }

    // --------------------------------------------------------------------------------------

    void UdpLogViewer::CopyAll()
    {
        m_model->MsgCopyAllLogToClipboard();
    }

    // --------------------------------------------------------------------------------------

    void UdpLogViewer::Clear()
    {
        // status cleared via model
        m_model->MsgClearDB();
    }

    // --------------------------------------------------------------------------------------

    void UdpLogViewer::Shutdown()
    {
        m_model->MsgShutdown();
    }

    // --------------------------------------------------------------------------------------

    void UdpLogViewer::PauseView(bool state)
    {
        m_model->MsgPauseView(state);
    }

    // --------------------------------------------------------------------------------------
    // private slots

    void UdpLogViewer::OnGuiUpdate(boost::shared_ptr<Ulv::ModelToViewParams> params)
    {
        ESS_ASSERT(InGuiThread());

        m_owner.Update(params);

        // time from last update
        int elapsedMs = (Platform::GetSystemMicrosecTickCount() - m_lastRefreshMcs) / 1000L;
        ESS_ASSERT(elapsedMs >= 0);

        int interval = (m_currTimerIntervalMs - elapsedMs) <= 0 ? 
            1 : (m_currTimerIntervalMs - elapsedMs);

        ESS_ASSERT(interval >= 1);
        m_refreshTimer.start(interval);
    }

    // --------------------------------------------------------------------------------------

    void UdpLogViewer::OnGuiUpdateStatus(Ulv::UdpLogDBStatus status)
    {
        m_owner.UpdateStatus(status);
    }

    // --------------------------------------------------------------------------------------

    void UdpLogViewer::OnGuiShowError(QString error, bool isCritical)
    {
        m_owner.ShowError(error, isCritical);
    }

    // --------------------------------------------------------------------------------------

    void UdpLogViewer::OnGuiSaveToClipboard(QString data)
    {
        QClipboard *p = QApplication::clipboard();
        p->setText(data);

        // std::cout << "Debug: Copied to clipboard" << std::endl;
    }

    /*void UdpLogViewer::OnGuiClose()
    {
        m_owner.Close();
    }*/

} // namespace UlvGui

