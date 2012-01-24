#ifndef __ULVGUISCROLLWIDGET__
#define __ULVGUISCROLLWIDGET__

#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"

class UlvGuiScrollWidget
    : public QScrollBar
{
    Q_OBJECT;

    void wheelEvent(QWheelEvent *event) // override
    {
        event->ignore();
    }

public:
    UlvGuiScrollWidget(QWidget *pParent = 0) : QScrollBar(pParent)
    {}

    ~UlvGuiScrollWidget()
    {}
};

#endif
