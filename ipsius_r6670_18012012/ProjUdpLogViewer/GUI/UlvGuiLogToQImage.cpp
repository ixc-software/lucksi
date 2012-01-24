
#include "stdafx.h"
#include "UlvGuiLogToQImage.h"
#include "UlvGuiImageRender.h"


// Render helpers
namespace
{
    using namespace UlvGui;
    using namespace Ulv;

    int CalcDirection(const ImageRender &renderer, bool scrollAtEnd, int inScrollPos, const UdpLogDB &db)
    {
        // int direction = ((scrollAtEnd) && (dbSize >= maxLines)) ? -1 : 1;
        int res = 1; // ---v

        int maxLinesCount = renderer.MaxLinesCount();

        if (scrollAtEnd)
        {
            res = -1; // -----^
            if (db.RecordCount() < maxLinesCount)
            {
                res = 1;
                // count records lines to adjust direction
                int linesCount = 0;
                for (size_t i = 0; i < db.RecordCount(); ++i)
                {
                    linesCount += renderer.RecordLinesCount(db.RecordView(i));
                    if (linesCount >= maxLinesCount)
                    {
                        res = -1;
                        break;
                    }
                }
            }
        }

        return res;
    }

} // namespace

// ------------------------------------------------------------------

namespace
{
    const bool CDebugReturnEmptyImg = false;

} // namespace

// ------------------------------------------------------------------

namespace UlvGui
{
    boost::shared_ptr<QImage> LogToQImage(const Ulv::UdpLogDB &db,
                                          const GuiCtrlToModelParams &inParams,
                                          LogToQImageOutParams &outParams)
    {
        int dbSize = db.RecordCount();

        // init (painter + image)
        ImageRender renderer(inParams);

        if (CDebugReturnEmptyImg) return renderer.get();

        if (dbSize == 0) return renderer.get();

        int maxLines = renderer.MaxLinesCount();
        bool scrollAtEnd = inParams.ScrollWasAtEnd();
        int inScrollPos = inParams.ScrollPos();        
        int direction = CalcDirection(renderer, scrollAtEnd, inScrollPos, db);

        int lineTopY = 0;
        int dbIndex = (((inScrollPos + maxLines) > dbSize) || (scrollAtEnd)) ? (dbSize - maxLines) : inScrollPos;
        if (dbIndex < 0) dbIndex = 0;

        outParams.StartRecIndex = dbIndex;
        outParams.MaxImageLinesCount = maxLines;

        if (direction < 0)
        {
            lineTopY = (maxLines - 1) * renderer.LineHeight();
            dbIndex = dbSize - 1;
        }

        outParams.RenderedRecordsCount = 0;
        int renderedLines = 0;        
        while ((renderedLines < maxLines) && (dbIndex < dbSize))
        {
            int recLines = renderer.DrawOneRecord(lineTopY, db.RecordView(dbIndex),
                                                  direction);

            lineTopY += direction * recLines * renderer.LineHeight();
            renderedLines += recLines;
            dbIndex += direction;

            ++outParams.RenderedRecordsCount;        
        }

        outParams.RenderedLinesCount = (renderedLines < maxLines)? renderedLines : maxLines;

        if (direction < 0) outParams.StartRecIndex = dbIndex + 1;

        return renderer.get();
    }    

} // namespace UlvGui
