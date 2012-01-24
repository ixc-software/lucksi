#ifndef __ULVGUILOGTOQIMAGE__
#define __ULVGUILOGTOQIMAGE__

#include "Utils/ErrorsSubsystem.h"
#include "UlvGuiCtrlToModelParams.h"
#include "Core/UdpLogDB.h"

namespace UlvGui
{
    struct LogToQImageOutParams
    {
        int StartRecIndex;
        int RenderedRecordsCount;
        int RenderedLinesCount;
        int MaxImageLinesCount;

        LogToQImageOutParams() :
            StartRecIndex(0), RenderedRecordsCount(0), RenderedLinesCount(0),
            MaxImageLinesCount(0)
        {
        }
    };

    // ------------------------------------------------------------------

    /* Using:
       1) render image (in the Model thread);
       2) inside YourWindow::paintEvent() (in the QThread):
            QPainter painter;
            painter.drawImage(0, 0, image); */

    boost::shared_ptr<QImage> LogToQImage(const Ulv::UdpLogDB &db,
                                          const GuiCtrlToModelParams &inParams,
                                          LogToQImageOutParams &outParams);



} // namespace UlvGui

#endif
