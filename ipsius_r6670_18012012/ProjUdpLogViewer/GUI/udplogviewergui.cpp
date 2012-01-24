#include "stdafx.h"

#include "udplogviewergui.h"
#include "ui_udplogviewergui.h"
#include "UdpLogViewerGuiHelpers.h"
#include "QtDeclaredMetaTypes.h"
#include "UlvGuiLogViewRender.h"

#include "Utils/ExeName.h"

#include "Core/UlvError.h"
#include "Core/UlvTypes.h"
#include "Core/UdpLogFilter.h"
#include "Core/UdpLogModel.h"


/*
  int id = qRegisterMetaType<boost::shared_ptr<Ulv::Status> >();
*/

namespace 
{
    using namespace Ulv;
    using namespace UlvGui;

    // Adjust scroll from Render() method
    // if scroll hasn't been moved by user for --v seconds
    const dword CScrollAdjustIntervalMs = 1 * 1000;

    // ----------------------------------------------------------------

    const QString CLogFontName = "Courier";
    const int CLogFontSize = 9;

    // ----------------------------------------------------------------

    const QString CDefaultConfigFile = "UdpLogViewerConfig.ini";

    // ----------------------------------------------------------------

    const QString CDontUseTimestamp = "None";
    const QString CUseTimestampRecvShort = "Receiver(short)";
    const QString CUseTimestampRecvLong = "Receiver(long)";
    const QString CUseTimestampSendShort = "Sender(short)";
    const QString CUseTimestampSendLong = "Sender(long)";

    // ----------------------------------------------------------------

    const int CMinFilterChars = 2;


} // namespace

// --------------------------------------------------------------------------------------------

UdpLogViewerGUI::UdpLogViewerGUI(const Ulv::StartupParams &params,
                                 const QString &buildInfo,
                                 QWidget *parent) :
    QMainWindow(parent),
    m_paramsInited(false),
    m_startTime(QDateTime::currentDateTime()),
    ui(new Ui::UdpLogViewerGUIClass),
    m_viewerImplParams(params),
    m_lastScrollMovingTime(QDateTime::currentDateTime()),
    m_errorsCount(0),
    m_modelClosed(false)
{
    m_debugLevel = 0;

    InitStartConfig();

    // init ui
    ui->setupUi(this);
    InitWidgets(params, buildInfo);

    // init params
    InitCtrlParams();
}

// --------------------------------------------------------------------------------------------

UdpLogViewerGUI::~UdpLogViewerGUI()
{
    delete ui;    
}

// --------------------------------------------------------------------------------------------

UlvGui::GuiCtrlToModelParams& UdpLogViewerGUI::CurrParams()
{    
    return m_currCtrlParams;
}

// --------------------------------------------------------------------------------------------
// UlvTests::IDebugSrcListWidgetEvents impl

void UdpLogViewerGUI::DebugError(const QString &error)
{
    ShowError(error, false);
}

// --------------------------------------------------------------------------------------------
// Config handlers

void UdpLogViewerGUI::InitStartConfig()
{
    QString fileNameAndPath = QString::fromStdString(Utils::ExeName::GetExeDir());
    fileNameAndPath += CDefaultConfigFile;
    try
    {
        m_startConfig.reset(new UlvGui::StartConfig(fileNameAndPath));
    }
    catch(UlvGui::StartConfig::StartConfigError &e)
    {
        CriticalError("Start config cannot be loaded. File I/O error.");
    }
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::UpdateStartConfigFile()
{
    ESS_ASSERT(m_startConfig.get() != 0);

    // m_startConfig->EnableUseRcvrTimestamp(CheckBoxStateToBool(*ui->m_useRcvrTimestamp));
    // m_startConfig->EnableUseSrcInLog(CheckBoxStateToBool(*ui->m_useSrcInLog));

    m_startConfig->ShowDebugBox(ui->m_debugBox->isVisible());
    m_startConfig->ShowInfoBox(ui->m_infoBox->isVisible());
    m_startConfig->ShowFilterBox(ui->m_filterBox->isVisible());
    m_startConfig->TimestampModeIndex(ui->m_timestampSelect->currentIndex());
    m_startConfig->FormWidth(this->width());
    m_startConfig->FormHeight(this->height());
    m_startConfig->FormTop(this->x());
    m_startConfig->FormLeft(this->y());
}

// --------------------------------------------------------------------------------------------
// Init handlers

void UdpLogViewerGUI::InitWidgets(const Ulv::StartupParams &params,
                                  const QString &buildInfo)
{
    DebugLvl1("InitWidgets");

    // scroll
    UpdateScroll(0, 0, 0);
    setMouseTracking(true);

    // log
    connect(ui->m_logView, SIGNAL(LogResize()), this, SLOT(OnLogResize()));

    // log src
    int id = qRegisterMetaType<Ulv::UdpLogRecordSrcInfoList>();
    ui->m_logSrcListGui->ShowInColors(CheckBoxStateToBool(*ui->m_srcInColor));
    connect(ui->m_logSrcListGui, SIGNAL(SelectionChanged(Ulv::UdpLogRecordSrcInfoList)),
            this, SLOT(OnSrcSelectionChanged(Ulv::UdpLogRecordSrcInfoList)));

    InitToolBar();
    InitDebugBox(params);
    InitFilterBox();
    InitShortcuts();
    InitMainWindowForm();
    InitInfo(params, buildInfo);

    // status bar
    UpdateStatus(Ulv::UdpLogDBStatus());

    ui->m_filter->setToolTip("2 non-space characters minimun, \n"
                             "use \"||\" for OR and \"&&\" for AND (has higher priority)");

    m_paramsInited = true;

    DebugLvl1("\\InitWidgets");
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::InitMainWindowForm()
{
    // init form apperance
    ESS_ASSERT(m_startConfig.get() != 0);
    // ui->m_useRcvrTimestamp->setChecked(m_startConfig->EnableUseRcvrTimestamp());
    // ui->m_useSrcInLog->setChecked(m_startConfig->EnableUseSrcInLog());
    m_debugBoxShowAct->setChecked(m_startConfig->ShowDebugBox());
    m_infoBoxShowAct->setChecked(m_startConfig->ShowInfoBox());
    m_filterBoxShowAct->setChecked(m_startConfig->ShowFilterBox());
    ui->m_debugBox->setVisible(m_startConfig->ShowDebugBox());
    ui->m_infoBox->setVisible(m_startConfig->ShowInfoBox());
    ui->m_filterBox->setVisible(m_startConfig->ShowFilterBox());
    int index = (m_startConfig->TimestampModeIndex() <= ui->m_timestampSelect->count())?
                m_startConfig->TimestampModeIndex() : ui->m_timestampSelect->count();
    ui->m_timestampSelect->setCurrentIndex(index);

    // set form min height
    int min = this->geometry().height();
    if (ui->m_debugBox->isHidden()) DecFormMinHeight(ui->m_debugBox->minimumHeight());
    if (ui->m_filterBox->isHidden()) DecFormMinHeight(ui->m_filterBox->minimumHeight());
    if (ui->m_infoBox->isHidden()) DecFormMinHeight(ui->m_infoBox->minimumHeight());

    // set geometry (geometry - form rect WITHOUT window title)
    int xDiff = this->geometry().x() - this->x();
    int yDiff = this->geometry().y() - this->y();
    QRect geometry(m_startConfig->FormTop() - xDiff, m_startConfig->FormLeft() - yDiff,
                   m_startConfig->FormWidth(), m_startConfig->FormHeight());
    this->setGeometry(geometry);
    this->move(m_startConfig->FormTop(), m_startConfig->FormLeft());

    // set icon
    this->setWindowIcon(QIcon(IconPath("app.ico")));
}

// --------------------------------------------------------------------------------------------

QAction* UdpLogViewerGUI::NewAction(const QString &name, const QString &iconPath,
                                    bool checkable, bool checked = false)
{
    QAction *p = new QAction(QIcon(iconPath), name, this);
    ESS_ASSERT(p != 0);

    if (!checkable) ESS_ASSERT(!checked);

    p->setCheckable(checkable);
    p->setChecked(checked);

    return p;
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::InitToolBar()
{
    ui->toolBar->setMovable(false);

    // works if only write in this order: create and init here, connect, add
    m_filterBoxShowAct = NewAction("&Filter(F5)", IconPath("open.png"), true, true);
    m_filterBoxShowAct->setShortcut(Qt::Key_F5);
    m_filterBoxShowAct->setToolTip("F5");
    connect(m_filterBoxShowAct, SIGNAL(triggered()), this, SLOT(OnFilterBoxShow()));
    ui->toolBar->addAction(m_filterBoxShowAct);

    m_debugBoxShowAct = NewAction("&Debug(F6)", IconPath("open.png"), true, true);
    m_debugBoxShowAct->setShortcut(Qt::Key_F6);
    m_debugBoxShowAct->setToolTip("F6");
    connect(m_debugBoxShowAct, SIGNAL(triggered()), this, SLOT(OnDebugBoxShow()));
    ui->toolBar->addAction(m_debugBoxShowAct);

    m_infoBoxShowAct = NewAction("&Info(F7)", IconPath("open.png"), true, true);
    m_infoBoxShowAct->setShortcut(Qt::Key_F7);
    m_infoBoxShowAct->setToolTip("F7");
    connect(m_infoBoxShowAct, SIGNAL(triggered()), this, SLOT(OnInfoBoxShow()));
    ui->toolBar->addAction(m_infoBoxShowAct);

    ui->toolBar->addSeparator();

    m_pauseViewAct = NewAction("&Pause view", IconPath("pause.png"), true, false);
    connect(m_pauseViewAct, SIGNAL(triggered()), this, SLOT(OnPauseView()));
    ui->toolBar->addAction(m_pauseViewAct);

    ui->toolBar->addSeparator();

    m_saveToFileAct = NewAction("&Save", IconPath("save.png"), false);
    connect(m_saveToFileAct, SIGNAL(triggered()), this, SLOT(OnSaveToFile()));
    ui->toolBar->addAction(m_saveToFileAct);

    m_copyAct = NewAction("&Copy", IconPath("copy.png"), false);
    connect(m_copyAct, SIGNAL(triggered()), this, SLOT(OnCopy()));
    ui->toolBar->addAction(m_copyAct);

    m_copyAllAct = NewAction("&Copy All", IconPath("copy.png"), false);
    connect(m_copyAllAct, SIGNAL(triggered()), this, SLOT(OnCopyAll()));
    ui->toolBar->addAction(m_copyAllAct);

    m_clearAct = NewAction("&Clear", IconPath("cut.png"), false);
    connect(m_clearAct, SIGNAL(triggered()), this, SLOT(OnClear()));
    ui->toolBar->addAction(m_clearAct);

    ui->toolBar->addSeparator();

    m_exitAct = NewAction("&Exit", IconPath("exit.png"), false);
    connect(m_exitAct, SIGNAL(triggered()), this, SLOT(OnExit()));
    ui->toolBar->addAction(m_exitAct);

    ui->toolBar->addSeparator();    
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::InitDebugBox(const Ulv::StartupParams &params)
{
    ui->m_debugSrcListGui->Setup(this, params.DBParams().Parser().UsedDateTimeFormat());

    // receiver ports
    for (size_t i = 0; i < params.PortsCount(); ++i)
    {
        QString text(QString::number(params.Port(i)));
        text += " (";
        text += PortTypeToString(params.PortDataType(i));
        text += ")";
        ui->m_debugRcvrPorts->addItem(text);
    }
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::InitFilterBox()
{
    ui->m_timestampSelect->addItem(CDontUseTimestamp);
    ui->m_timestampSelect->addItem(CUseTimestampRecvShort);
    ui->m_timestampSelect->addItem(CUseTimestampRecvLong);
    ui->m_timestampSelect->addItem(CUseTimestampSendShort);
    ui->m_timestampSelect->addItem(CUseTimestampSendLong);

    ui->m_timestampSelect->setCurrentIndex(0);
}

// --------------------------------------------------------------------------------------------



void UdpLogViewerGUI::InitShortcuts()
{
    m_logPgUp = new QShortcut(Qt::Key_PageUp, this);
    m_logPgDown = new QShortcut(Qt::Key_PageDown, this);

    connect(m_logPgUp, SIGNAL(activated()), this, SLOT(OnPageUp()));
    connect(m_logPgDown, SIGNAL(activated()), this, SLOT(OnPageDown()));
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::InitInfo(const Ulv::StartupParams &params, const QString &buildInfo)
{
    ShowInfo(buildInfo);
    if (params.DBParams().RecordingEnabled())
    {
        ShowInfo(QString("Log is recording to %1").arg(params.DBParams().RecordingTo()));
    }
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::InitCtrlParams()
{
    // gui
    CurrParams().LogFont(CLogFontName, CLogFontSize);

    CurrParams().LogWindow(ui->m_logView->width(), ui->m_logView->height());
    CurrParams().IgnoreLogRecordLF(CheckBoxStateToBool(*ui->m_ignoreRecLF));

    // common
    CurrParams().RecordViewWithSrc(CheckBoxStateToBool(*ui->m_useSrcInLog));
    CurrParams().RecordViewTimestamp(MakeTimestampView(*ui->m_timestampSelect));

    FilterExp filter(ui->m_filter->text(), CheckBoxStateToBool(*ui->m_filterCase));
    CurrParams().Filter(filter);
}

// --------------------------------------------------------------------------------------------
// Overrided methods

void UdpLogViewerGUI::paintEvent (QPaintEvent * event) // override
{
    if (m_viewerImpl != 0) return;

    m_viewerImpl.reset(new UlvGui::UdpLogViewer(*this, m_viewerImplParams, m_modelClosed));
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::closeEvent (QCloseEvent *event) // override
{
    ui->m_debugSrcListGui->Clear();

    // exit on socker bind error or other error in constructor
    if (!m_viewerImpl) return;

    m_viewerImpl->Shutdown();

    while(!m_modelClosed)
    {
        Platform::ThreadSleep(100);
    }

    UpdateStartConfigFile(); // before form hide
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::resizeEvent (QResizeEvent *event) // override
{
    OnLogResize();
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::wheelEvent(QWheelEvent *event) // override
{
    if ((childAt(event->pos()) != ui->m_logView)
        && (childAt(event->pos()) != ui->m_logScroll))
    {
        event->ignore();
        return;
    }

    //   Qt: Most mouse types work in steps of 15 degrees,
    //       in which case the delta value is a multiple of 120; i.e., 120 * 1/8 = 15.

    int numSteps = (event->delta() / 8) / 15;
    numSteps *= ui->m_logScroll->pageStep();

    int newPos = ui->m_logScroll->value() - numSteps;

    on_m_logScroll_sliderMoved(newPos);

    event->accept();
}

// --------------------------------------------------------------------------------------------
// Toolbar widgets slots

void UdpLogViewerGUI::OnClear()
{
    // reset all related to DB

    // scroll
    UpdateScroll(0, 0, 0);

    // log src
    ui->m_logSrcListGui->Clear();


    // log
    ui->m_logView->Clear();


    m_viewerImpl->Clear();
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::OnSaveToFile()
{
    m_viewerImpl->SaveToFile();
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::OnCopy()
{
    int startRecord = ui->m_logScroll->value();
    int count = ui->m_logScroll->pageStep();
    m_viewerImpl->Copy(startRecord, count);
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::OnCopyAll()
{
    m_viewerImpl->CopyAll();
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::OnExit()
{
    this->close();
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::OnDebugBoxShow()
{
    if (ui->m_debugBox->isHidden())
    {
        ui->m_debugBox->setVisible(true);
        IncFormMinHeight(ui->m_debugBox->height());
        return;
    }

    ui->m_debugSrcListGui->Clear();

    ui->m_debugBox->setVisible(false);
    DecFormMinHeight(ui->m_debugBox->height());
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::OnFilterBoxShow()
{
    if (ui->m_filterBox->isHidden())
    {
        ui->m_filterBox->setVisible(true);
        IncFormMinHeight(ui->m_filterBox->height());
        ui->m_filter->setFocus();
        return;
    }

    ui->m_filterBox->setVisible(false);
    DecFormMinHeight(ui->m_filterBox->height());
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::OnInfoBoxShow()
{
    if (ui->m_infoBox->isHidden())
    {
        ui->m_infoBox->setVisible(true);
        IncFormMinHeight(ui->m_infoBox->height());
        return;
    }

    ui->m_infoBox->setVisible(false);
    DecFormMinHeight(ui->m_infoBox->height());
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::OnPauseView()
{
    m_viewerImpl->PauseView(m_pauseViewAct->isChecked());
}

// --------------------------------------------------------------------------------------------
// Other widgets slots

void UdpLogViewerGUI::on_m_srcInColor_stateChanged(int )
{
    if (!m_paramsInited) return;

    // update gui src list
    ui->m_logSrcListGui->ShowInColors(CheckBoxStateToBool(*ui->m_srcInColor));

    CurrParams().SrcSett(ui->m_logSrcListGui->Sett());
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::on_m_filter_textChanged(QString value)
{
    if (!m_paramsInited) return;

    value = value.trimmed();

    if ((value.size() > 0) && (value.size() < CMinFilterChars)) return;

    FilterChangedByUser();
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::on_BDebugAddSrc_clicked()
{
    bool convertionOK = false;
    QString portStr = ui->m_debugRcvrPorts->currentText().split(" ").at(0);
    int port = portStr.toInt(&convertionOK);
    if (!convertionOK)
    {
        ShowError("Debug: invalid port number", false);
        return;
    }

    convertionOK = false;
    int packsPerSec = ui->m_debugPacksPerSec->currentText().toInt(&convertionOK);
    if ((!convertionOK) || (packsPerSec <= 0))
    {
        ShowError("Debug: invalid intensity", false);
        return;
    }

    UdpPortDataType dataType = (ui->m_debugDataType->isChecked())? PTWstring : PTString;

    ui->m_debugSrcListGui->AddAndRun(port, packsPerSec, dataType);
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::on_m_useSrcInLog_stateChanged(int )
{
    if (!m_paramsInited) return;

    CurrParams().RecordViewWithSrc(CheckBoxStateToBool(*ui->m_useSrcInLog));    
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::on_m_logScroll_valueChanged(int value)
{
    DebugLvl1("on_m_logScroll_valueChanged");

    UpdateScroll(value, ui->m_logScroll->maximum(), ui->m_logScroll->pageStep());    
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::on_m_logScroll_sliderMoved(int position)
{
    DebugLvl1("on_m_logScroll_sliderMoved");

    if (position < 0) position = 0;

    if (position == ui->m_logScroll->value()) return;

    m_lastScrollMovingTime = QDateTime::currentDateTime();

    UpdateScroll(position, ui->m_logScroll->maximum(), ui->m_logScroll->pageStep());    
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::on_m_timestampSelect_currentIndexChanged(int index)
{
    if (!m_paramsInited) return;

    CurrParams().RecordViewTimestamp(MakeTimestampView(*ui->m_timestampSelect));    
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::on_m_filterCase_stateChanged(int )
{
    if (!m_paramsInited) return;

    FilterChangedByUser();
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::on_m_ignoreRecLF_stateChanged(int )
{
    if (!m_paramsInited) return;

    CurrParams().IgnoreLogRecordLF(CheckBoxStateToBool(*ui->m_ignoreRecLF));
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::OnPageUp()
{
    // ESS_ASSERT(0);
    int pos = ui->m_logScroll->value();
    pos -= ui->m_logScroll->pageStep();
    if (pos < 0) pos = 0;

    on_m_logScroll_sliderMoved(pos);
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::OnPageDown()
{
    int pos = ui->m_logScroll->value();
    pos += ui->m_logScroll->pageStep();
    if (pos > ui->m_logScroll->maximum()) pos = ui->m_logScroll->maximum();

    on_m_logScroll_sliderMoved(pos);
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::OnSrcSelectionChanged(Ulv::UdpLogRecordSrcInfoList list)
{
    CurrParams().SrcList(list);
    CurrParams().SrcSett(ui->m_logSrcListGui->Sett());
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::OnLogResize()
{
    CurrParams().LogWindow(ui->m_logView->width(), ui->m_logView->height());
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::IncFormMinHeight(int val)
{
    this->setMinimumHeight(this->minimumHeight() + val);
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::DecFormMinHeight(int val)
{
    this->setMinimumHeight(this->minimumHeight() - val);
    ESS_ASSERT(this->minimumHeight() > ui->m_logBox->minimumHeight());
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::FilterChangedByUser()
{
    DebugLvl1("FilterChangedByUser");

    FilterExp filter(ui->m_filter->text(), CheckBoxStateToBool(*ui->m_filterCase));
    CurrParams().Filter(filter);
}

// --------------------------------------------------------------------------------------------

bool UdpLogViewerGUI::IsScrollWasAtEnd() const
{
    return (ui->m_logScroll->value() == ui->m_logScroll->maximum());
}

// --------------------------------------------------------------------------------------------

bool UdpLogViewerGUI::IsScrollParamsChanged(int pos, int size, int pageStep)
{
    return ((ui->m_logScroll->value() != pos)
            || (ui->m_logScroll->maximum() != size)
            || (ui->m_logScroll->pageStep() != pageStep));
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::UpdateScroll(int pos, int size, int pageStep)
{
    // if (!IsScrollParamsChanged(pos, size, pageStep)) return;
    
    DebugLvl1(QString("UpdateScroll %1 of %2, ps %3").arg(pos).arg(size).arg(pageStep));
    ui->m_logScroll->setMaximum(size);
    ui->m_logScroll->setValue(pos);
    ui->m_logScroll->setPageStep(pageStep);

    CurrParams().Scroll(ui->m_logScroll->value(), IsScrollWasAtEnd());
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::ScrollMovedByModel(int pos, int size, int pageStep)
{
    DebugLvl1(QString("ScrollMovedByModel %1 of %2, ps %3").arg(pos).arg(size).arg(pageStep));

    int addSize = size - ui->m_logScroll->maximum() - ui->m_logScroll->pageStep();

    bool scrollPosChanged = IsScrollParamsChanged(pos, size, pageStep);
    bool isTimeElapsed = TimeElapsed(m_lastScrollMovingTime, CScrollAdjustIntervalMs);

    // if pos reset to 0, ignore timer and update
    // if (!scrollPosChanged || !isTimeElapsed) return;

    if ((scrollPosChanged && isTimeElapsed) || (scrollPosChanged && (pos == 0)))
    {
        // update
        if (pageStep == 0)
        {
            UpdateScroll(0, 0, 0);
            return;
        }

        int addPageStep = pageStep - ui->m_logScroll->pageStep();
        int max = ui->m_logScroll->maximum() + addSize - addPageStep;
        if (max < 0) max = 0; // fix
        int realPos = (IsScrollWasAtEnd() /*&& (pos != 0)*/) ? max : pos;

        UpdateScroll(realPos, max, pageStep);
    }
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::DebugLvl1(const QString &text) const
{
    if (m_debugLevel != 1) return;

    std::cout << "Debug [UdpLogViewerGUI]: " << text.toStdString() << std::endl;
}

// --------------------------------------------------------------------------------------------
// UlvGui::IUdpLogViewerGui impl

bool UdpLogViewerGUI::IsVisible() const
{
    return this->isVisible();
}

// --------------------------------------------------------------------------------------------

bool UdpLogViewerGUI::IsMinimized() const
{
    return this->isMinimized();
}

// --------------------------------------------------------------------------------------------

bool UdpLogViewerGUI::IsNotInFocus() const
{
    return !this->isActiveWindow();
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::Update(boost::shared_ptr<Ulv::ModelToViewParams> params)
{
    DebugLvl1("Update");

    ESS_ASSERT(params != 0);

    // update log
    Ulv::RenderResult *pRes = params->RenderRes().get();
    ESS_ASSERT(pRes != 0);
    UlvGui::GuiRenderResult *pImg = dynamic_cast<UlvGui::GuiRenderResult*>(pRes);
    ESS_ASSERT(pImg != 0);
    ui->m_logView->Update(pImg->Get());

    // update scroll
    int newPageStep = pRes->RecordsDisplayed();
    if (pRes->UsedLinesCount() < pRes->MaxLinesCount()) newPageStep = 0;

    int size = (pRes->RecordsAll() == 0) ? 0 : (pRes->RecordsAll() - 1);
    ScrollMovedByModel(pRes->StartRecord(), size, newPageStep);

    // add/update src list
    bool newAdded = false;
    ui->m_logSrcListGui->Update(params->SrcList(), newAdded);

    if (newAdded)
    {
        CurrParams().SrcSett(ui->m_logSrcListGui->Sett());    
    }

    DebugLvl1("\\Update");
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::UpdateStatus(Ulv::UdpLogDBStatus status)
{
    // DebugLvl1("UpdateStatus");

    // status bar
    QString statusText = PacketsStatus(status.ReceivedPacksCount(), status.ReceivedBytes(),
                                       status.ReceivedInvalidPacksCount(), status.PacksInDBCount(),
                                       status.DroppedPackCount(), status.FilteredCount(),
                                       ui->m_filterBox->isVisible());
    statusText += DropStatus(status.DropTimes());
    statusText += RecStatus(status.RecTimes(), status.RecFileSize());
    statusText += ErrorsStatus(m_errorsCount);
    // if (m_recSett.Enable) statusText += "[Recording]";
    statusText += TimeStatus(m_startTime);
    ui->statusBar->showMessage(statusText);

    // status label
    statusText.clear();
    if (ui->m_debugBox->isVisible())
    {
        statusText += DebugStatus(ui->m_debugSrcListGui->ActiveCount(), ui->m_debugSrcListGui->Count());

        statusText += ScrollStatus(*ui->m_logScroll);
    }
    statusText += RenderStatus(status.RenderingImgCount(), status.RenderingImgAgvTimeMs(),
                               ui->m_logView->PaintCount(),
                               status.RefreshMsgCountAll(), status.RefreshMsgCountProcessed());
    ui->m_statusAdv->setText(statusText);

    // DebugLvl1("\\UpdateStatus");
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::ShowInfo(const QString &text)
{
    ui->m_info->addItem(text);
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::ShowError(const QString &err, bool isCritical)
{
    if (isCritical)
    {
        std::cerr << err.toStdString() << std::endl;
        // ShowErrorImpl(this, err);
        this->close();

        return;
    }

    // update error list
    ++m_errorsCount;
    QString text("Error (");
    text += QString::number(m_errorsCount);
    text += "): ";
    text += err;
    ui->m_info->addItem(text);
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::UpdateDebug()
{
    if (!ui->m_debugBox->isVisible()) return;

    ui->m_debugSrcListGui->Refresh();
}

// --------------------------------------------------------------------------------------------

/*void UdpLogViewerGUI::Close()
{
    m_waitCloseConfirm = false;
    this->close();
}*/

// --------------------------------------------------------------------------------------------

boost::shared_ptr<UlvGui::GuiCtrlToModelParams> UdpLogViewerGUI::CurrentParams() const
{
    // return copy
    boost::shared_ptr<UlvGui::GuiCtrlToModelParams> res;
    res.reset(new UlvGui::GuiCtrlToModelParams(m_currCtrlParams));

    return res;
}

// --------------------------------------------------------------------------------------------

void UdpLogViewerGUI::Show()
{
    this->show();
    ui->toolBar->show();
    UpdateStatus(Ulv::UdpLogDBStatus());
}

