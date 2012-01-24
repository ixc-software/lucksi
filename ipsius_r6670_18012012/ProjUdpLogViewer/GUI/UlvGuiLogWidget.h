#ifndef __ULVGUILOGWIDGET__
#define __ULVGUILOGWIDGET__

#include "Core/UdpLogRecord.h"

namespace Ulv
{
    class RenderResult;

} // namespace Ulv

// --------------------------------------------------

class LogWidget :
    public QWidget
{
    Q_OBJECT;

    boost::shared_ptr<QImage> m_img;
    
    int m_paintCount;

private:
    void paintEvent(QPaintEvent *event); // override
    void resizeEvent(QResizeEvent *event); // override
    
signals:
    void LogResize();

public:
    LogWidget(QWidget *parent = 0);

    void Update(boost::shared_ptr<QImage> img);
    void Clear();

    int PaintCount() const { return m_paintCount; }
};

#endif


