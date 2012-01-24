
#include "stdafx.h"
#include "UdpLogModel.h"
#include "Core/UlvUtils.h"
#include "Platform/Platform.h"


namespace
{
    using namespace Ulv;

    const dword CNewDataRefreshIntervalMs = 200;

    const dword CCopyToClipboardTimeout = 5 * 60 * 1000;

} // namespace

// -------------------------------------------------------------------------

namespace Ulv
{
    UdpLogModel::UdpLogModel(iCore::MsgThread &thread,
                             const StartupParams &params,
                             Utils::SafeRef<IModelToView> view,
                             IUdpLogViewRender *pViewRender,
                             Utils::AtomicBool &exit) :
        iCore::MsgObject(thread),
        m_state(StInit),
        m_params(params),
        m_db(params.DBParams(), thread),
        m_udpMonitor(thread, *this),        
        m_viewRender(pViewRender),
        m_view(view),
        m_dbDropped(false),
        m_exit(exit),
        m_renderCounter(0)
    {
        m_debugLevel = 0;

        ESS_ASSERT(!m_view.IsEmpty());
        ESS_ASSERT(m_viewRender != 0);

        for (size_t i = 0; i < m_params.PortsCount(); ++i)
        {
            m_udpMonitor.AddPort(m_params.Port(i), m_params.PortDataType(i));
        }
    }

    // -------------------------------------------------------------------------
 
    UdpLogModel::~UdpLogModel()
    {
        if (!m_view.IsEmpty()) m_view.Clear();        
    }

    // -------------------------------------------------------------------------

    void UdpLogModel::DebugLvl1(const QString &text)
    {
        if (m_debugLevel != 1) return;

        std::cout << "Debug [UdpLogModel] "
                << QDateTime::currentDateTime().toString().toStdString()
                << ": "<< text.toStdString() << std::endl;
    }

    // -------------------------------------------------------------------------

    void UdpLogModel::DebugLvl2(const QString &text)
    {
        if (m_debugLevel != 2) return;

        std::cout << "Debug [UdpLogModel]"
                /*<< QDateTime::currentDateTime().toString().toStdString()*/
                << ": "<< text.toStdString() << std::endl;
    }

    // -------------------------------------------------------------------------
    // IUdpMonitorEvents impl

    void UdpLogModel::NewData(const QString &data, const Utils::HostInf &src)
    {
        ESS_ASSERT(m_state == StActive);

        // int oldSrcCount = m_db.RecordSrcList().Size();

        bool partOfDBWasDropped = false;
        bool packOk = m_db.Store(data, src, partOfDBWasDropped);

        m_dbDropped |= partOfDBWasDropped;

        /*if ((oldSrcCount != m_db.RecordSrcList().Size()) || partOfDBWasDropped)
        {
            DebugLvl2("NewData: refresh");
            m_currCtrlParams->SrcList(m_db.RecordSrcList());
            OnRefreshView(m_currCtrlParams);
            DebugLvl2("\\NewData: refresh");
        }*/
    }

    // -------------------------------------------------------------------------

    void UdpLogModel::ErrorOccur(boost::shared_ptr<Error> error)
    {
        // ESS_ASSERT(m_state == StActive);
        if (m_state == StExit)
        {
            DebugLvl1("Error occured while inactive.");
            return;
        }

        m_view->DisplayError(*error);
    }

    // -------------------------------------------------------------------------
    // events

    void UdpLogModel::OnPauseView(bool state)
    {
        m_db.IgnoreNewRecordsOnGet(state);
    }

    // -------------------------------------------------------------------------

    void UdpLogModel::OnShutdown()
    {
        DebugLvl2("OnShutdown");

        ESS_ASSERT(m_state != StExit);
        DebugLvl1("Shutting down.");

        m_state = StExit;

        m_udpMonitor.Close();
        m_db.Clear(); // save remained data to file

        m_exit.Set(true);

        DebugLvl2("\\OnShutdown");
    }

    // -------------------------------------------------------------------------
    
    void UdpLogModel::OnCopyLogToClipboard(CopyParams params)
    {
        if (m_state != StActive) return;

        // m_clipboardData = m_db.GetCopy(params.StartRecord, params.Count);
        // params.Event->Ready();

        m_view->SaveToClipboard(m_db.GetCopy(params.StartRecord, params.Count));

    }

    // -------------------------------------------------------------------------
    
    void UdpLogModel::OnSaveLogToFile(QString fileName)
    {
        if (m_state != StActive) return;
        
        QString fileErr;
        // m_db.SaveToFile(fileName, 0, m_db.RecordCount(), fileErr);

        UlvUtils::SaveToFile(fileName, m_db.GetCopy(0, m_db.RecordCount()),
                             false, fileErr);

        if (fileErr.isEmpty()) return;

        m_view->DisplayError(FileIOError(fileErr));
    }

   // -------------------------------------------------------------------------

    void UdpLogModel::OnClearDB()
    {
        if (m_state != StActive) return;
        DebugLvl2("OnClearDB");

        m_db.Clear();

        // OnRefreshView(m_currCtrlParams);
        m_dbDropped = true;

        DebugLvl2("\\ OnClearDB");
    }

    // -------------------------------------------------------------------------

//    void UdpLogModel::OnRefreshView(boost::shared_ptr<CtrlToModelParams> params)
//    {
//        DebugLvl2("OnRefreshView");
//
//        ESS_ASSERT(params != 0);
//
//        if (m_state == StExit) return;
//
//        if (m_state == StInit)
//        {
//            m_state = StActive;
//            m_currRenderParams = params;
//            return;
//        }
//
//        if (m_waitRenderParamsUpdate)
//        {
//            if (params->AllSrcCount() < m_db.SrcCount()) return; // wait sync
//            m_waitRenderParamsUpdate = false;
//        }
//
//        bool scrollPosChanged = (CurrRenderParams().ScrollPos() != params->ScrollPos());
//
//        bool dbRefiltered = false;
//
//        // sync src
//        if (CurrRenderParams().SelectedSrcList() != params->SelectedSrcList())
//        {
//            m_db.SyncRecordSrcSelection(params->SelectedSrcList());
//            dbRefiltered = true;
//        }
//
//        // update view settings, refilter
//        dbRefiltered |= m_db.UpdateSettings(params->RecordView());
//
//        // set and update current params
//        m_currRenderParams = params;
//        if (dbRefiltered) CurrRenderParams().Scroll(0, false);
//
//        // render
//        bool dbSizeChanged = (m_lastRenderedDBSize != m_db.RecordCount());
//        m_lastRenderedDBSize = m_db.RecordCount();
//
//        if ((CurrRenderParams().ScrollWasAtEnd() && dbSizeChanged)
//            || scrollPosChanged
//            || dbRefiltered)
//        {
//            UpdateView();
//        }
//
//        m_view->UpdateStatus(m_db.Status());
//
//        DebugLvl2("\\OnRefreshView");
//    }

    void UdpLogModel::OnRefreshView(/*boost::shared_ptr<CtrlToModelParams> params*/)
    {
        DebugLvl2("OnRefreshView");

        if (m_state == StInit)
        {
            m_state = StActive;
            return;
        }

        if (m_state == StExit) return;

        if (m_refreshParams.empty()) return;


        int refreshListSize = m_refreshParams.size();
        boost::shared_ptr<CtrlToModelParams> last = m_refreshParams.back();


        bool dbRefiltered = false;

        // sync src
        dbRefiltered = m_db.SyncRecordSrcSelection(last->SrcList());

        // update view settings, refilter
        dbRefiltered |= m_db.UpdateSettings(last->RecordView());

        // modify input m_lastRefreshParams
        if (dbRefiltered || m_dbDropped)
        {
            m_dbDropped = false;
            last->Scroll(0, false);            
        }


        // render
        ++ m_renderCounter;
        ddword before = Platform::GetSystemTickCount();

        boost::shared_ptr<RenderResult> res = m_viewRender->Render(m_db, last.get());

        ddword after = Platform::GetSystemTickCount();

        m_renderTime.Add(after - before);


        // make params
        boost::shared_ptr<ModelToViewParams> toViewParams;
        toViewParams.reset(new ModelToViewParams(res, m_db.RecordSrcList().InfoList()));

        if (dbRefiltered) ESS_ASSERT(toViewParams->RenderRes()->StartRecord() == 0);

        // update
        m_view->Update(toViewParams);

        // update status
        m_db.UpdateStatus(m_refreshMsgCount, m_renderCounter, m_renderTime.Average());
        m_view->UpdateStatus(m_db.Status());

        // clear param list        
        for (int i = 0; i < refreshListSize; ++i)
        {
            m_refreshParams.pop();
        }

        DebugLvl2("\\OnRefreshView");
    }

    // -------------------------------------------------------------------------
    // IControllerToModel impl

    void UdpLogModel::MsgRefreshView(boost::shared_ptr<CtrlToModelParams> params)
    {
        DebugLvl2("MsgRefreshView");

        m_refreshParams.push(params);
        m_refreshMsgCount.Inc();

        PutMsg(this, &T::OnRefreshView);

        DebugLvl2("\\MsgRefreshView");
    }

    // -------------------------------------------------------------------------

    void UdpLogModel::MsgPauseView(bool state)
    {
        ESS_ASSERT(m_state == StActive);

        PutMsg(this, &T::OnPauseView, state);
    }

    // -------------------------------------------------------------------------

    void UdpLogModel::MsgShutdown()
    {
        ESS_ASSERT(m_state != StExit);

        PutMsg(this, &T::OnShutdown);
    }

    // -------------------------------------------------------------------------

    void UdpLogModel::MsgCopyAllLogToClipboard()
    {
        ESS_ASSERT(m_state == StActive);

        MsgCopyDisplayedLogToClipboard(0, -1);
    }

    // -------------------------------------------------------------------------

    void UdpLogModel::MsgCopyDisplayedLogToClipboard(int startRecord, int count)
    {
        ESS_ASSERT(m_state == StActive);

        CopyParams p(startRecord, count);

        PutMsg(this, &T::OnCopyLogToClipboard, p);

        /*if (!p.Event->Wait(CCopyToClipboardTimeout))
        {
            ESS_ASSERT(0 && "Wait coping to clipboard fail!");
        }

        QClipboard *pCb = QApplication::clipboard();
        pCb->setText(m_clipboardData);        */
    }

    // -------------------------------------------------------------------------

    void UdpLogModel::MsgSaveLogToFile(QString fileName)
    {
        ESS_ASSERT(m_state == StActive);

        PutMsg(this, &T::OnSaveLogToFile, fileName);
    }

    // -------------------------------------------------------------------------

    void UdpLogModel::MsgClearDB()
    {
        ESS_ASSERT(m_state == StActive);

        PutMsg(this, &T::OnClearDB);
    }        

} // namespace Ulv
