
#include "stdafx.h"

#include "UlvGuiLogWidget.h"
#include "UlvGuiLogViewRender.h"


// -------------------------------------------------------------------

LogWidget::LogWidget(QWidget *parent) :
    QWidget(parent), m_paintCount(0)
{    
}

// -------------------------------------------------------------------

void LogWidget::Update(boost::shared_ptr<QImage> img)
{
    ESS_ASSERT(img != 0);

    m_img = img;

    this->update();
}

// -------------------------------------------------------------------

void LogWidget::Clear()
{
    m_img.reset();

    this->update();
}

// -------------------------------------------------------------------

void LogWidget::paintEvent(QPaintEvent *event) // override
{
    if (m_img == 0) return;

    ++m_paintCount;

    QPainter painter(this);
    painter.drawImage(0, 0, *m_img);
}

// -------------------------------------------------------------------

void LogWidget::resizeEvent(QResizeEvent *event) // override
{
    emit LogResize();
}



