#ifndef UDPLOGVIEWERGUIHELPERS_H
#define UDPLOGVIEWERGUIHELPERS_H

#include "stdafx.h"

#include "Core/UlvTypes.h"

// --------------------------------------------------------------------

namespace UlvGui
{
    using Ulv::dword;
    using Ulv::ddword;

    // converters
    bool CheckBoxStateToBool(const QCheckBox &box);
    QString PortTypeToString(Ulv::UdpPortDataType type);
    Ulv::TimestampView MakeTimestampView(const QComboBox &timestampBox);

    // status
    QString PacketsStatus(ddword all, ddword bytes, ddword invalid, dword inDB, ddword dropped,
                          dword filtered, bool showFiltered);
    QString ErrorsStatus(ddword count);
    QString DebugStatus(dword activeCount, dword allCount);
    QString ScrollStatus(const QScrollBar &scroll);
    QString TimeStatus(const QDateTime &startTime);
    QString RenderStatus(ddword imgRendered, ddword imgRenderingAvgTime, ddword imgPainted,
                         ddword refreshMsgAll, ddword refreshMsgProcessed);
    QString DropStatus(ddword times);
    QString RecStatus(ddword times, ddword fileSize);

    // other
    void ShowErrorImpl(QWidget *pOwner, const QString &err);
    bool TimeElapsed(const QDateTime &startDT, Ulv::dword mSec);
    QString IconPath(const QString &fileName);


} // namespace UlvGui


#endif // UDPLOGVIEWERGUIHELPERS_H
