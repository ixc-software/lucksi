#ifndef __UDPLOGVIEWERGUI__
#define __UDPLOGVIEWERGUI__


#include "Tests/DebugSrcList.h"

#include "UlvGuiStartConfig.h"
#include "UlvGuiCtrlToModelParams.h"
#include "UdpLogViewer.h"

#include "Core/UlvStartupParams.h"
#include "Core/UdpLogDBStatus.h"
#include "Core/UdpLogRecordSrc.h"
#include "Core/ModelToViewParams.h"

namespace Ui
{
    class UdpLogViewerGUIClass;
}

// ---------------------------------------------

// Main GUI class
class UdpLogViewerGUI :
    /*public*/ QMainWindow,
    public UlvGui::IUdpLogViewerGui,
    public UlvTests::IDebugErrorHandler
{
    Q_OBJECT;

    typedef UdpLogViewerGUI T;

    // UlvGui::IGuiControllerToController &m_owner;

    bool m_paramsInited;
    QDateTime m_startTime;
    
    Ui::UdpLogViewerGUIClass *ui;

    UlvGui::GuiCtrlToModelParams m_currCtrlParams;

    // gui startup config
    boost::scoped_ptr<UlvGui::StartConfig> m_startConfig; // can throw from constructor

    // viewer impl
    boost::scoped_ptr<UlvGui::UdpLogViewer> m_viewerImpl; // to create inside paint event
    const Ulv::StartupParams &m_viewerImplParams;

    // counters & states
    QDateTime m_lastScrollMovingTime;
    Ulv::ddword m_errorsCount;
    Utils::AtomicBool m_modelClosed;

    int m_debugLevel;

    // toolbar actions
    QAction *m_saveToFileAct;
    QAction *m_copyAct;
    QAction *m_copyAllAct;
    QAction *m_clearAct;
    QAction *m_exitAct;
    QAction *m_filterBoxShowAct;
    QAction *m_infoBoxShowAct;
    QAction *m_debugBoxShowAct;
    QAction *m_pauseViewAct;

    // shortcuts
    QShortcut *m_logPgUp;
    QShortcut *m_logPgDown;

// UlvGui::IUdpLogViewerGui impl
private:
    bool IsVisible() const;
    bool IsMinimized() const;
    bool IsNotInFocus() const;

    void ShowError(const QString &err, bool isCritical);
    void ShowInfo(const QString &text);

    // void Close();

    void Update(boost::shared_ptr<Ulv::ModelToViewParams> params);
    void UpdateStatus(Ulv::UdpLogDBStatus status);
    void UpdateDebug();

    boost::shared_ptr<UlvGui::GuiCtrlToModelParams> CurrentParams() const;

// UlvTests::IDebugSrcListWidgetEvents impl
private:
    void DebugError(const QString &error);

private:
    void paintEvent (QPaintEvent * event); // override
    void closeEvent (QCloseEvent * event); // override    
    void resizeEvent (QResizeEvent *event); // override
    void wheelEvent(QWheelEvent *event); // override

private:
    UlvGui::GuiCtrlToModelParams& CurrParams();

    // init    
    void InitWidgets(const Ulv::StartupParams &params,
                     const QString &buildInfo);
    QAction* NewAction(const QString &name, const QString &iconPath,
                       bool checkable, bool checked);
    void InitToolBar();
    void InitDebugBox(const Ulv::StartupParams &params);
    void InitFilterBox();
    void InitShortcuts();
    void InitMainWindowForm();
    void InitInfo(const Ulv::StartupParams &params, const QString &buildInfo);
    void InitCtrlParams();
    void InitStartConfig();
    void UpdateStartConfigFile(); // call from destructor

    void IncFormMinHeight(int val);
    void DecFormMinHeight(int val);

    bool IsScrollWasAtEnd() const;
    bool IsScrollParamsChanged(int size, int pos, int displayedRecCount);

    void ScrollMovedByModel(int pos, int size, int pageStep);
    void UpdateScroll(int pos, int size, int pageStep);

    void FilterChangedByUser();

    void DebugLvl1(const QString &text) const;

private slots:
    void on_m_logScroll_valueChanged(int value);
    void on_m_ignoreRecLF_stateChanged(int );
    void on_m_srcInColor_stateChanged(int );

    void on_m_filterCase_stateChanged(int );
    void on_m_timestampSelect_currentIndexChanged(int index);


    void on_m_useSrcInLog_stateChanged(int );
    void on_BDebugAddSrc_clicked();
    void on_m_logScroll_sliderMoved(int position);
    void on_m_filter_textChanged(QString value);

    // on toolbar's button
    void OnClear();
    void OnSaveToFile();
    void OnCopy();
    void OnCopyAll();
    void OnExit();

    // on toolbar's button checked / unchecked
    void OnDebugBoxShow();
    void OnFilterBoxShow();
    void OnInfoBoxShow();
    void OnPauseView();

    // shortcuts handlers
    void OnPageUp();
    void OnPageDown();

    // src list handlers
    void OnSrcSelectionChanged(Ulv::UdpLogRecordSrcInfoList list);

    // other 
    void OnLogResize();

public:
    UdpLogViewerGUI(const Ulv::StartupParams &params,
                    const QString &buildInfo,
                    QWidget *parent = 0);
    ~UdpLogViewerGUI();

    void Show();
};

#endif



