#ifndef __ULVGUILOGVIEWGUISRCLIST__
#define __ULVGUILOGVIEWGUISRCLIST__


#include "Utils/ErrorsSubsystem.h"

#include "Core/UdpLogRecordSrc.h"

#include "UlvGuiCtrlToModelParams.h"



// UdpLogRecord sources GUI list. Sources checkable.
class LogSrcWidget :
    public QListWidget
{
    Q_OBJECT;

    Ulv::UdpLogRecordSrcInfoList m_list;
    bool m_showInColors;

    // context menu actions
    QAction *m_logSrcSelectAll;
    QAction *m_logSrcDeselectAll;

    void Add(const Ulv::UdpLogRecordSrcInfo &src);
    QListWidgetItem* Find(const Ulv::UdpLogRecordSrcInfo &src) const;
    QColor Color(int index) const;
    QString Name(int index) const;

private slots:
    void OnMenuSelectAll();
    void OnMenuUnselectAll();

    void OnCustomContextMenuRequested(QPoint pos);
    void OnItemClicked(QListWidgetItem *pItem);

signals:
    void SelectionChanged(Ulv::UdpLogRecordSrcInfoList list);

public:
    LogSrcWidget(QWidget *parent = 0);
    ~LogSrcWidget();

    void Update(const Ulv::UdpLogRecordSrcInfoList &list, bool &newAdded);
    void ShowInColors(bool state);
    void Clear();

    UlvGui::RecordSrcGuiSettList Sett() const;
};


#endif
