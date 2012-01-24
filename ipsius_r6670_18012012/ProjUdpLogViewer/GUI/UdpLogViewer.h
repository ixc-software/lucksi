#ifndef UDPLOGVIEWER_H
#define UDPLOGVIEWER_H

#include "Utils/AtomicTypes.h"


// #include "udplogviewergui.h"
#include "IUdpLogViewerGui.h"
#include "UlvGuiLogViewRender.h"

#include "Core/IUdpLogModelToView.h"
#include "Core/IUdpLogControllerToModel.h"
#include "Core/UdpLogDB.h"
#include "Core/UlvError.h"
#include "Core/ModelToViewParams.h"
#include "Core/CtrlToModelParams.h"
#include "Core/UlvStartupParams.h"

// ---------------------------------------------

namespace UlvGui
{
    // Log viewer main class
    class UdpLogViewer :
            public QObject,
            public virtual Utils::SafeRefServer,
            public Ulv::IModelToView
    {
        Q_OBJECT;

        int m_guiThreadId;

        // main window
        IUdpLogViewerGui &m_owner;

        iCore::MsgThread m_modelThread;
        boost::scoped_ptr<Ulv::IControllerToModel> m_model;

        QTimer m_refreshTimer; // refresh view
        int m_currTimerIntervalMs;
        Ulv::ddword m_lastRefreshMcs;
        Utils::AtomicBool m_modelThreadIdSet;


    // Ulv::IModelToView
    private:
        void Update(boost::shared_ptr<Ulv::ModelToViewParams> params);
        void UpdateStatus(Ulv::UdpLogDBStatus status);
        void DisplayError(Ulv::Error error);
        void SaveToClipboard(QString data);

    private:
        void ConnectSigSlots();
        bool InGuiThread() const;

    private slots:
        void OnGuiUpdate(boost::shared_ptr<Ulv::ModelToViewParams> params);
        void OnGuiUpdateStatus(Ulv::UdpLogDBStatus status);
        void OnGuiShowError(QString error, bool isCritical);
        void OnGuiSaveToClipboard(QString data);
        // void OnGuiClose();
        void OnTimerRefresh();
        void OnHasModelThreadId(int id);

    signals:
        void GuiUpdate(boost::shared_ptr<Ulv::ModelToViewParams> params);
        void GuiUpdateStatus(Ulv::UdpLogDBStatus status);
        void GuiShowError(QString error, bool isCritical);
        void GuiSaveToClipboard(QString data);
        // void GuiClose();
        void HasModelThreadId(int id);

    public:
        UdpLogViewer(IUdpLogViewerGui &owner,
                     const Ulv::StartupParams &params,
                     Utils::AtomicBool &modelClosed);

        ~UdpLogViewer();

        void PauseView(bool state);
        void Shutdown();
        void Clear();

        void SaveToFile();
        void Copy(int startRecord, int count);
        void CopyAll();
    };

} // namespace UlvGui

#endif



