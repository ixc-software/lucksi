/********************************************************************************
** Form generated from reading UI file 'udplogviewergui.ui'
**
** Created: Mon Oct 25 18:10:21 2010
**      by: Qt User Interface Compiler version 4.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UDPLOGVIEWERGUI_H
#define UI_UDPLOGVIEWERGUI_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "GUI/UlvGuiLogSrcWidget.h"
#include "GUI/UlvGuiLogWidget.h"
#include "GUI/UlvGuiScrollWidget.h"
#include "Tests/DebugSrcList.h"

QT_BEGIN_NAMESPACE

class Ui_UdpLogViewerGUIClass
{
public:
    QAction *actionSafe_log_to_file;
    QAction *actionExit;
    QAction *actionClear_log;
    QAction *actionStatus_menu;
    QAction *actionError_window;
    QAction *actionStatus_window;
    QAction *actionError_window_2;
    QAction *actionSettings;
    QAction *actionRun;
    QAction *actionStop;
    QAction *actionSettings_2;
    QAction *actionRun_2;
    QAction *actionStop_2;
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout_5;
    QGroupBox *m_filterBox;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_5;
    QVBoxLayout *verticalLayout;
    QLineEdit *m_filter;
    QHBoxLayout *horizontalLayout_3;
    QCheckBox *m_filterCase;
    QSpacerItem *horizontalSpacer_4;
    QCheckBox *m_srcInColor;
    QHBoxLayout *horizontalLayout_4;
    QCheckBox *m_useSrcInLog;
    QLabel *label;
    QComboBox *m_timestampSelect;
    QSpacerItem *horizontalSpacer_5;
    QCheckBox *m_ignoreRecLF;
    QSpacerItem *horizontalSpacer_3;
    QSpacerItem *horizontalSpacer_2;
    LogSrcWidget *m_logSrcListGui;
    QGroupBox *m_debugBox;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_3;
    QComboBox *m_debugRcvrPorts;
    QLabel *label_4;
    QComboBox *m_debugPacksPerSec;
    QCheckBox *m_debugDataType;
    QPushButton *BDebugAddSrc;
    QSpacerItem *horizontalSpacer;
    UlvTests::DebugSrcListWidget *m_debugSrcListGui;
    QGroupBox *m_infoBox;
    QVBoxLayout *verticalLayout_4;
    QListWidget *m_info;
    QLabel *m_statusAdv;
    QGroupBox *m_logBox;
    QVBoxLayout *verticalLayout_7;
    QHBoxLayout *horizontalLayout;
    LogWidget *m_logView;
    UlvGuiScrollWidget *m_logScroll;
    QStatusBar *statusBar;
    QToolBar *toolBar;

    void setupUi(QMainWindow *UdpLogViewerGUIClass)
    {
        if (UdpLogViewerGUIClass->objectName().isEmpty())
            UdpLogViewerGUIClass->setObjectName(QString::fromUtf8("UdpLogViewerGUIClass"));
        UdpLogViewerGUIClass->setWindowModality(Qt::NonModal);
        UdpLogViewerGUIClass->resize(780, 783);
        UdpLogViewerGUIClass->setMinimumSize(QSize(780, 783));
        QFont font;
        font.setPointSize(8);
        UdpLogViewerGUIClass->setFont(font);
        UdpLogViewerGUIClass->setMouseTracking(true);
        UdpLogViewerGUIClass->setContextMenuPolicy(Qt::DefaultContextMenu);
        UdpLogViewerGUIClass->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        actionSafe_log_to_file = new QAction(UdpLogViewerGUIClass);
        actionSafe_log_to_file->setObjectName(QString::fromUtf8("actionSafe_log_to_file"));
        actionExit = new QAction(UdpLogViewerGUIClass);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        actionClear_log = new QAction(UdpLogViewerGUIClass);
        actionClear_log->setObjectName(QString::fromUtf8("actionClear_log"));
        actionStatus_menu = new QAction(UdpLogViewerGUIClass);
        actionStatus_menu->setObjectName(QString::fromUtf8("actionStatus_menu"));
        actionError_window = new QAction(UdpLogViewerGUIClass);
        actionError_window->setObjectName(QString::fromUtf8("actionError_window"));
        actionStatus_window = new QAction(UdpLogViewerGUIClass);
        actionStatus_window->setObjectName(QString::fromUtf8("actionStatus_window"));
        actionStatus_window->setCheckable(true);
        actionStatus_window->setChecked(true);
        actionError_window_2 = new QAction(UdpLogViewerGUIClass);
        actionError_window_2->setObjectName(QString::fromUtf8("actionError_window_2"));
        actionError_window_2->setCheckable(true);
        actionError_window_2->setChecked(true);
        actionSettings = new QAction(UdpLogViewerGUIClass);
        actionSettings->setObjectName(QString::fromUtf8("actionSettings"));
        actionRun = new QAction(UdpLogViewerGUIClass);
        actionRun->setObjectName(QString::fromUtf8("actionRun"));
        actionStop = new QAction(UdpLogViewerGUIClass);
        actionStop->setObjectName(QString::fromUtf8("actionStop"));
        actionSettings_2 = new QAction(UdpLogViewerGUIClass);
        actionSettings_2->setObjectName(QString::fromUtf8("actionSettings_2"));
        actionRun_2 = new QAction(UdpLogViewerGUIClass);
        actionRun_2->setObjectName(QString::fromUtf8("actionRun_2"));
        actionStop_2 = new QAction(UdpLogViewerGUIClass);
        actionStop_2->setObjectName(QString::fromUtf8("actionStop_2"));
        actionStop_2->setEnabled(false);
        centralWidget = new QWidget(UdpLogViewerGUIClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        verticalLayout_5 = new QVBoxLayout(centralWidget);
        verticalLayout_5->setSpacing(6);
        verticalLayout_5->setContentsMargins(11, 11, 11, 11);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        m_filterBox = new QGroupBox(centralWidget);
        m_filterBox->setObjectName(QString::fromUtf8("m_filterBox"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(m_filterBox->sizePolicy().hasHeightForWidth());
        m_filterBox->setSizePolicy(sizePolicy);
        m_filterBox->setMinimumSize(QSize(0, 131));
        verticalLayout_2 = new QVBoxLayout(m_filterBox);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        m_filter = new QLineEdit(m_filterBox);
        m_filter->setObjectName(QString::fromUtf8("m_filter"));
        m_filter->setFocusPolicy(Qt::WheelFocus);
        m_filter->setEchoMode(QLineEdit::Normal);
        m_filter->setCursorPosition(0);

        verticalLayout->addWidget(m_filter);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        m_filterCase = new QCheckBox(m_filterBox);
        m_filterCase->setObjectName(QString::fromUtf8("m_filterCase"));

        horizontalLayout_3->addWidget(m_filterCase);

        horizontalSpacer_4 = new QSpacerItem(138, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_4);

        m_srcInColor = new QCheckBox(m_filterBox);
        m_srcInColor->setObjectName(QString::fromUtf8("m_srcInColor"));
        m_srcInColor->setChecked(true);

        horizontalLayout_3->addWidget(m_srcInColor);


        verticalLayout->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        m_useSrcInLog = new QCheckBox(m_filterBox);
        m_useSrcInLog->setObjectName(QString::fromUtf8("m_useSrcInLog"));

        horizontalLayout_4->addWidget(m_useSrcInLog);

        label = new QLabel(m_filterBox);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_4->addWidget(label);

        m_timestampSelect = new QComboBox(m_filterBox);
        m_timestampSelect->setObjectName(QString::fromUtf8("m_timestampSelect"));
        sizePolicy.setHeightForWidth(m_timestampSelect->sizePolicy().hasHeightForWidth());
        m_timestampSelect->setSizePolicy(sizePolicy);
        m_timestampSelect->setMinimumSize(QSize(0, 0));

        horizontalLayout_4->addWidget(m_timestampSelect);

        horizontalSpacer_5 = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_5);

        m_ignoreRecLF = new QCheckBox(m_filterBox);
        m_ignoreRecLF->setObjectName(QString::fromUtf8("m_ignoreRecLF"));
        m_ignoreRecLF->setEnabled(true);
        m_ignoreRecLF->setCheckable(true);
        m_ignoreRecLF->setChecked(true);

        horizontalLayout_4->addWidget(m_ignoreRecLF);

        horizontalSpacer_3 = new QSpacerItem(18, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_3);


        verticalLayout->addLayout(horizontalLayout_4);


        horizontalLayout_5->addLayout(verticalLayout);

        horizontalSpacer_2 = new QSpacerItem(18, 17, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_2);

        m_logSrcListGui = new LogSrcWidget(m_filterBox);
        m_logSrcListGui->setObjectName(QString::fromUtf8("m_logSrcListGui"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(m_logSrcListGui->sizePolicy().hasHeightForWidth());
        m_logSrcListGui->setSizePolicy(sizePolicy1);
        m_logSrcListGui->setMinimumSize(QSize(300, 81));
        m_logSrcListGui->setMaximumSize(QSize(16777215, 81));
        m_logSrcListGui->setContextMenuPolicy(Qt::CustomContextMenu);

        horizontalLayout_5->addWidget(m_logSrcListGui);


        verticalLayout_2->addLayout(horizontalLayout_5);


        verticalLayout_5->addWidget(m_filterBox);

        m_debugBox = new QGroupBox(centralWidget);
        m_debugBox->setObjectName(QString::fromUtf8("m_debugBox"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Minimum);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(m_debugBox->sizePolicy().hasHeightForWidth());
        m_debugBox->setSizePolicy(sizePolicy2);
        m_debugBox->setMinimumSize(QSize(0, 181));
        m_debugBox->setMaximumSize(QSize(16777215, 181));
        verticalLayout_3 = new QVBoxLayout(m_debugBox);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_3 = new QLabel(m_debugBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_2->addWidget(label_3);

        m_debugRcvrPorts = new QComboBox(m_debugBox);
        m_debugRcvrPorts->setObjectName(QString::fromUtf8("m_debugRcvrPorts"));

        horizontalLayout_2->addWidget(m_debugRcvrPorts);

        label_4 = new QLabel(m_debugBox);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout_2->addWidget(label_4);

        m_debugPacksPerSec = new QComboBox(m_debugBox);
        m_debugPacksPerSec->setObjectName(QString::fromUtf8("m_debugPacksPerSec"));
        m_debugPacksPerSec->setEditable(true);

        horizontalLayout_2->addWidget(m_debugPacksPerSec);

        m_debugDataType = new QCheckBox(m_debugBox);
        m_debugDataType->setObjectName(QString::fromUtf8("m_debugDataType"));
        m_debugDataType->setChecked(false);

        horizontalLayout_2->addWidget(m_debugDataType);

        BDebugAddSrc = new QPushButton(m_debugBox);
        BDebugAddSrc->setObjectName(QString::fromUtf8("BDebugAddSrc"));

        horizontalLayout_2->addWidget(BDebugAddSrc);

        horizontalSpacer = new QSpacerItem(118, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);


        verticalLayout_3->addLayout(horizontalLayout_2);

        m_debugSrcListGui = new UlvTests::DebugSrcListWidget(m_debugBox);
        m_debugSrcListGui->setObjectName(QString::fromUtf8("m_debugSrcListGui"));
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(m_debugSrcListGui->sizePolicy().hasHeightForWidth());
        m_debugSrcListGui->setSizePolicy(sizePolicy3);
        m_debugSrcListGui->setMinimumSize(QSize(0, 101));
        m_debugSrcListGui->setMaximumSize(QSize(16777215, 101));
        m_debugSrcListGui->setContextMenuPolicy(Qt::CustomContextMenu);

        verticalLayout_3->addWidget(m_debugSrcListGui);


        verticalLayout_5->addWidget(m_debugBox);

        m_infoBox = new QGroupBox(centralWidget);
        m_infoBox->setObjectName(QString::fromUtf8("m_infoBox"));
        sizePolicy2.setHeightForWidth(m_infoBox->sizePolicy().hasHeightForWidth());
        m_infoBox->setSizePolicy(sizePolicy2);
        m_infoBox->setMinimumSize(QSize(0, 165));
        m_infoBox->setMaximumSize(QSize(16777215, 165));
        m_infoBox->setMouseTracking(true);
        verticalLayout_4 = new QVBoxLayout(m_infoBox);
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setContentsMargins(11, 11, 11, 11);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        m_info = new QListWidget(m_infoBox);
        m_info->setObjectName(QString::fromUtf8("m_info"));
        QSizePolicy sizePolicy4(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(m_info->sizePolicy().hasHeightForWidth());
        m_info->setSizePolicy(sizePolicy4);
        m_info->setMinimumSize(QSize(0, 101));
        m_info->setMaximumSize(QSize(16777215, 101));

        verticalLayout_4->addWidget(m_info);

        m_statusAdv = new QLabel(m_infoBox);
        m_statusAdv->setObjectName(QString::fromUtf8("m_statusAdv"));

        verticalLayout_4->addWidget(m_statusAdv);


        verticalLayout_5->addWidget(m_infoBox);

        m_logBox = new QGroupBox(centralWidget);
        m_logBox->setObjectName(QString::fromUtf8("m_logBox"));
        m_logBox->setMinimumSize(QSize(468, 210));
        m_logBox->setMouseTracking(false);
        verticalLayout_7 = new QVBoxLayout(m_logBox);
        verticalLayout_7->setSpacing(6);
        verticalLayout_7->setContentsMargins(11, 11, 11, 11);
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        m_logView = new LogWidget(m_logBox);
        m_logView->setObjectName(QString::fromUtf8("m_logView"));
        m_logView->setMinimumSize(QSize(716, 169));
        m_logView->setMouseTracking(false);

        horizontalLayout->addWidget(m_logView);

        m_logScroll = new UlvGuiScrollWidget(m_logBox);
        m_logScroll->setObjectName(QString::fromUtf8("m_logScroll"));
        QSizePolicy sizePolicy5(QSizePolicy::Fixed, QSizePolicy::Minimum);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(m_logScroll->sizePolicy().hasHeightForWidth());
        m_logScroll->setSizePolicy(sizePolicy5);
        m_logScroll->setMinimumSize(QSize(0, 100));
        m_logScroll->setMouseTracking(true);
        m_logScroll->setSingleStep(10);
        m_logScroll->setPageStep(10);
        m_logScroll->setOrientation(Qt::Vertical);

        horizontalLayout->addWidget(m_logScroll);


        verticalLayout_7->addLayout(horizontalLayout);


        verticalLayout_5->addWidget(m_logBox);

        UdpLogViewerGUIClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(UdpLogViewerGUIClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        UdpLogViewerGUIClass->setStatusBar(statusBar);
        toolBar = new QToolBar(UdpLogViewerGUIClass);
        toolBar->setObjectName(QString::fromUtf8("toolBar"));
        toolBar->setEnabled(true);
        toolBar->setMinimumSize(QSize(0, 0));
        QFont font1;
        font1.setFamily(QString::fromUtf8("Sans Serif"));
        font1.setPointSize(8);
        toolBar->setFont(font1);
        toolBar->setContextMenuPolicy(Qt::CustomContextMenu);
        toolBar->setMovable(false);
        toolBar->setAllowedAreas(Qt::TopToolBarArea);
        toolBar->setIconSize(QSize(24, 24));
        toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        toolBar->setFloatable(false);
        UdpLogViewerGUIClass->addToolBar(Qt::TopToolBarArea, toolBar);

        retranslateUi(UdpLogViewerGUIClass);

        m_timestampSelect->setCurrentIndex(-1);
        m_debugPacksPerSec->setCurrentIndex(3);


        QMetaObject::connectSlotsByName(UdpLogViewerGUIClass);
    } // setupUi

    void retranslateUi(QMainWindow *UdpLogViewerGUIClass)
    {
        UdpLogViewerGUIClass->setWindowTitle(QApplication::translate("UdpLogViewerGUIClass", "UdpLogViewerGUI", 0, QApplication::UnicodeUTF8));
        actionSafe_log_to_file->setText(QApplication::translate("UdpLogViewerGUIClass", "Safe log to file", 0, QApplication::UnicodeUTF8));
        actionExit->setText(QApplication::translate("UdpLogViewerGUIClass", "Exit", 0, QApplication::UnicodeUTF8));
        actionClear_log->setText(QApplication::translate("UdpLogViewerGUIClass", "Clear log", 0, QApplication::UnicodeUTF8));
        actionStatus_menu->setText(QApplication::translate("UdpLogViewerGUIClass", "Status window", 0, QApplication::UnicodeUTF8));
        actionError_window->setText(QApplication::translate("UdpLogViewerGUIClass", "Error window", 0, QApplication::UnicodeUTF8));
        actionStatus_window->setText(QApplication::translate("UdpLogViewerGUIClass", "Status window", 0, QApplication::UnicodeUTF8));
        actionError_window_2->setText(QApplication::translate("UdpLogViewerGUIClass", "Error window", 0, QApplication::UnicodeUTF8));
        actionSettings->setText(QApplication::translate("UdpLogViewerGUIClass", "Settings ...", 0, QApplication::UnicodeUTF8));
        actionRun->setText(QApplication::translate("UdpLogViewerGUIClass", "Run", 0, QApplication::UnicodeUTF8));
        actionStop->setText(QApplication::translate("UdpLogViewerGUIClass", "Stop", 0, QApplication::UnicodeUTF8));
        actionSettings_2->setText(QApplication::translate("UdpLogViewerGUIClass", "Settings ...", 0, QApplication::UnicodeUTF8));
        actionRun_2->setText(QApplication::translate("UdpLogViewerGUIClass", "Run", 0, QApplication::UnicodeUTF8));
        actionStop_2->setText(QApplication::translate("UdpLogViewerGUIClass", "Stop", 0, QApplication::UnicodeUTF8));
        m_filterBox->setTitle(QApplication::translate("UdpLogViewerGUIClass", "Filter and view:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        m_filter->setToolTip(QApplication::translate("UdpLogViewerGUIClass", "Enter 2 characters minimum, use \"||\" for OR", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        m_filterCase->setText(QApplication::translate("UdpLogViewerGUIClass", "Case sensitive", 0, QApplication::UnicodeUTF8));
        m_srcInColor->setText(QApplication::translate("UdpLogViewerGUIClass", "Sources in color", 0, QApplication::UnicodeUTF8));
        m_useSrcInLog->setText(QApplication::translate("UdpLogViewerGUIClass", "Source name", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("UdpLogViewerGUIClass", "TS:", 0, QApplication::UnicodeUTF8));
        m_ignoreRecLF->setText(QApplication::translate("UdpLogViewerGUIClass", "Ignore record LF", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_WHATSTHIS
        m_debugBox->setWhatsThis(QApplication::translate("UdpLogViewerGUIClass", "Choose port", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_WHATSTHIS
        m_debugBox->setTitle(QApplication::translate("UdpLogViewerGUIClass", "Debug sources:", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("UdpLogViewerGUIClass", "Port:", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("UdpLogViewerGUIClass", "Intensity (packets/sec):", 0, QApplication::UnicodeUTF8));
        m_debugPacksPerSec->clear();
        m_debugPacksPerSec->insertItems(0, QStringList()
         << QApplication::translate("UdpLogViewerGUIClass", "1", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("UdpLogViewerGUIClass", "2", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("UdpLogViewerGUIClass", "5", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("UdpLogViewerGUIClass", "10", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("UdpLogViewerGUIClass", "20", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("UdpLogViewerGUIClass", "50", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("UdpLogViewerGUIClass", "100", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("UdpLogViewerGUIClass", "500", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("UdpLogViewerGUIClass", "1000", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        m_debugPacksPerSec->setToolTip(QString());
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        m_debugPacksPerSec->setStatusTip(QString());
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        m_debugPacksPerSec->setWhatsThis(QString());
#endif // QT_NO_WHATSTHIS
        m_debugDataType->setText(QApplication::translate("UdpLogViewerGUIClass", "UNICODE", 0, QApplication::UnicodeUTF8));
        BDebugAddSrc->setText(QApplication::translate("UdpLogViewerGUIClass", "Add && Run", 0, QApplication::UnicodeUTF8));
        m_infoBox->setTitle(QApplication::translate("UdpLogViewerGUIClass", "Info:", 0, QApplication::UnicodeUTF8));
        m_statusAdv->setText(QApplication::translate("UdpLogViewerGUIClass", "<Advanced status>", 0, QApplication::UnicodeUTF8));
        m_logBox->setTitle(QApplication::translate("UdpLogViewerGUIClass", "Log:", 0, QApplication::UnicodeUTF8));
        toolBar->setWindowTitle(QString());
    } // retranslateUi

};

namespace Ui {
    class UdpLogViewerGUIClass: public Ui_UdpLogViewerGUIClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_UDPLOGVIEWERGUI_H
