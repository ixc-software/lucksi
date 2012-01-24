
#include "stdafx.h"

#include "UlvGuiImageRender.h"

// constants
namespace
{
    const QImage::Format CImageFormat = QImage::Format_RGB32;

} // namespace

// ------------------------------------------------------------------

namespace
{
    using namespace UlvGui;

    // Ranges for current char in text view
    class CurrentRanges
    {
        typedef std::vector<Ulv::UdpLogRecordViewRange> List;
        List m_ranges;

    public:

        // true on changes
        bool Enter(const Ulv::UdpLogRecordViewRangesList &ranges, int pos)
        {
            bool changes = false;
            for(size_t i = 0; i < ranges.Size(); ++i)
            {
                if (ranges[i].Enter(pos))
                {
                    m_ranges.push_back(ranges[i]);
                    changes = true;
                }
            }

            return changes;
        }

        // true on changes
        bool Leave(int pos)
        {
            bool changes = false;

            List::iterator i = m_ranges.begin();
            while(i != m_ranges.end())
            {
                if (i->Leave(pos))
                {
                    i = m_ranges.erase(i); // returns pointer to element after erased
                    changes = true;
                    continue;
                }

                ++i;
            }




            return changes;
        }

        void Apply(const RangeGuiSettFactory &factory, RenderAttributes &attr)
        {
            for(size_t i = 0; i < m_ranges.size(); ++i)
            {
                factory.Apply(attr, m_ranges[i].RangeType());
            }
        }
    };
}

// ------------------------------------------------------------------

namespace UlvGui
{
    // ------------------------------------------------------------------
    // RangeGuiSett impl

    RangeGuiSett::RangeGuiSett(Ulv::UdpLogRecordViewRange::Type type, const QString &sett) :
        m_type(type)
    {
        // ...
        ESS_HALT("Not implemented yet");
    }

    // -------------------------------------------------------------------

    RangeGuiSett::RangeGuiSett(Ulv::UdpLogRecordViewRange::Type type,
                               const QColor &fontColor, const QColor &fontBGColor) :
        m_type(type),
        m_useFontColor(true), m_fontColor(fontColor),
        m_useFontBGColor(true), m_fontBGColor(fontBGColor)
    {
    }

    // -------------------------------------------------------------------

    void RangeGuiSett::Apply(RenderAttributes &attr,
                             Ulv::UdpLogRecordViewRange::Type type) const
    {
        if (type != m_type) return;

        if (m_useFontColor) attr.FontColor(m_fontColor);
        if (m_useFontBGColor) attr.FontBGColor(m_fontBGColor);
    }


    // -------------------------------------------------------------------
    // RangesStyles impl

    RangeGuiSettFactory::RangeGuiSettFactory()
    {
        typedef Ulv::UdpLogRecordViewRange RecRange;

        // register styles
        m_list.push_back(RangeGuiSett(RecRange::RTSrc,             0x15892A,  Qt::transparent));
        m_list.push_back(RangeGuiSett(RecRange::RTRecvrTimestamp,  0x1E18CB,  Qt::transparent));
        m_list.push_back(RangeGuiSett(RecRange::RTSenderTimestamp, 0x733BAE,  Qt::transparent));
        m_list.push_back(RangeGuiSett(RecRange::RTData,            Qt::black, Qt::transparent));
        m_list.push_back(RangeGuiSett(RecRange::RTSessionName,     Qt::black, Qt::transparent));
        m_list.push_back(RangeGuiSett(RecRange::RTSessionTag,      Qt::black, Qt::transparent));
        m_list.push_back(RangeGuiSett(RecRange::RTFilter,          Qt::black, Qt::yellow));
    }

    // -------------------------------------------------------------------

    void RangeGuiSettFactory::Apply(RenderAttributes &attr,
                                    Ulv::UdpLogRecordViewRange::Type type) const
    {
        for(size_t i = 0; i < m_list.size(); ++i)
        {
            m_list.at(i).Apply(attr, type); // do nothing if not same type
        }
    }

    // -------------------------------------------------------------------
    // RenderGuiSettings impl

    RenderGuiSettings::RenderGuiSettings(const GuiCtrlToModelParams &sett) :
            m_default(sett.LogFontName(), sett.LogFontSize(), Qt::white),
            m_recSrcSettList(sett.SrcSett()),
            m_ignoreRecLF(sett.IgnoreLogRecordLF())
    {

    }

    // -------------------------------------------------------------------

    // set background depend on src
    void RenderGuiSettings::SetForSource(RenderAttributes &attr,
                                         const QString &srcName) const
    {
        if (m_recSrcSettList.Size() == 0) return;

        QColor bg = m_recSrcSettList.GetColor(srcName);
        attr.FontBGColor(bg);
    }

    // ------------------------------------------------------------------
    // ImageRender impl

    ImageRender::ImageRender(const GuiCtrlToModelParams &sett) :
            m_image(new QImage(sett.LogWindowWidth(), sett.LogWindowHeight(), CImageFormat)),
            m_painter(m_image.get()), m_sett(sett)
    {
        // to fix default black background
        // m_painter.fillRect(m_image->rect(), m_sett.DefaultAttr().BG());
        DrawBG(m_image->rect(), m_sett.DefaultAttr().BG());
        m_painter.setFont(m_sett.DefaultAttr().Font());
    }

    // --------------------------------------------------------------------------

    int ImageRender::LineHeight() const
    {
        return m_painter.fontMetrics().height();
    }

    // --------------------------------------------------------------------------

    int ImageRender::LineCharCount(int startX) const
    {
        ESS_ASSERT(startX >= 0);

        int charWidthPix = m_painter.fontMetrics().boundingRect("A").width();
        ESS_ASSERT(charWidthPix > 0);

        int res = ((m_painter.window().width() - startX) / charWidthPix);
        ESS_ASSERT(res != 0);

        return res;
    }

    // --------------------------------------------------------------------------

    int ImageRender::MaxLinesCount() const
    {
        int maxHeight = m_painter.window().height();
        int minRecHeight = LineHeight();
        ESS_ASSERT(minRecHeight > 0);

        int res =  maxHeight / minRecHeight;
        // res += ((maxHeight % minRecHeight) < 9) ? 0 : 1;

        return res;
    }

    // --------------------------------------------------------------------------

    // returns number of lines rendered
    int ImageRender::DrawOneRecord(int lineTopY, const Ulv::UdpLogRecordView &rec, int direction)
    {
        ESS_ASSERT((lineTopY >= 0) && (lineTopY < m_painter.window().height()));
        ESS_ASSERT(direction != 0);

        std::vector<int> lines;
        RecordLines(rec, lines);
        ESS_ASSERT(!lines.empty());
        
        int recSize = rec.Get().size();
        int lineHeight = LineHeight();
        QPoint lineTopLeft(0, lineTopY);
        int startIndex = (direction > 0) ? 0 : (recSize - lines.back());
        int lineIndex = (direction > 0) ? 0 : (lines.size() - 1);

        int linesRendered = 0;
        int bottomBorder =  m_painter.window().height();
        while ((lineTopLeft.y() >= 0)
                && (lineTopLeft.y() <= bottomBorder)
                && (linesRendered < lines.size()))
        {
            int currLineCharCount = lines.at(lineIndex);

            Draw(lineTopLeft, m_sett, rec, startIndex, currLineCharCount);

            lineTopLeft.setY(lineTopLeft.y() + direction * lineHeight);

            ++linesRendered;
            lineIndex += direction;

            int nextLineCharCount = (lineIndex < lines.size())? lines.at(lineIndex) : 0;
            startIndex += (direction > 0 ) ? currLineCharCount : -nextLineCharCount;
        }

        return linesRendered;
    }

    // --------------------------------------------------------------------------

    void ImageRender::RecordLines(const Ulv::UdpLogRecordView &rec,
                                  std::vector<int> &linesSizes) const
    {
        linesSizes.clear();

        if (m_sett.IgnoreLogRecordLF())
        {
            AppendRecordLines(rec.Get().size(), linesSizes);
            return;
        }

        for (int i = 0; i < rec.LinesSizes().size(); ++i)
        {
            int curr = rec.LinesSizes().at(i);
            AppendRecordLines(rec.LinesSizes().at(i), linesSizes);
        }
    }

    // -----------------------------------------------------------------

    void ImageRender::AppendRecordLines(int textSize, std::vector<int> &linesSizes) const
    {
        int maxLineChars = LineCharCount(0);

        int lines = textSize / maxLineChars;
        int lastLineSize = textSize % maxLineChars;
        if (lastLineSize > 0) ++lines;
        else lastLineSize = maxLineChars;

        while(lines > 0)
        {
            int size = (lines == 1)? lastLineSize : maxLineChars;
            linesSizes.push_back(size);
            --lines;
        }
    }

    // -----------------------------------------------------------------

    int ImageRender::RecordLinesCount(const Ulv::UdpLogRecordView &rec) const
    {
        std::vector<int> lines;
        RecordLines(rec, lines);

        return lines.size();
    }

    // -----------------------------------------------------------------

    int ImageRender::DrawString(const QPoint &drawPos, const QString &s,
                              const RenderAttributes &currAttr)
    {
        QString text(s);
        QRect rect = m_painter.boundingRect(drawPos.x(), drawPos.y(),
                                            m_painter.window().width(), -1,
                                            Qt::AlignLeft, text);

        ESS_ASSERT(rect.height() == LineHeight());

        DrawBG(rect, currAttr.BG());

        // draw text
        m_painter.setPen(currAttr.Pen());
        m_painter.drawText (rect, Qt::AlignLeft, text);

        return rect.width();
    }

    // -----------------------------------------------------------------

    void ImageRender::DrawBG(const QRect &rect, const QBrush &bg, int borderSize)
    {
        // draw background
        QPalette palette;

        /*
        void qDrawShadeRect ( QPainter * painter, const QRect & rect,
                              const QPalette & palette, bool sunken = false, int lineWidth = 1,
                              int midLineWidth = 0, const QBrush * fill = 0 )*/
        qDrawShadeRect(&m_painter, rect, palette, false, 0, borderSize, &bg);
    }

    // -----------------------------------------------------------------

    void ImageRender::DrawLineBG(const QPoint &topLeft, const QBrush &bg)
    {
        int height =  m_painter.fontMetrics().height();
        QPoint bottomRight(m_painter.window().width(), (topLeft.y() + height));

        DrawBG(QRect(topLeft, bottomRight), bg, 1);
    }

    // -----------------------------------------------------------------

    void ImageRender::Draw(const QPoint &startDrawPosition,
                           const RenderGuiSettings &sett,
                           const Ulv::UdpLogRecordView &rec,
                           int startRecOffs, int recPosCount)
    {
        ESS_ASSERT((startRecOffs >= 0) && (recPosCount >= 0));
        ESS_ASSERT((startRecOffs + recPosCount) <= rec.Get().size());

        RenderAttributes defAttr = sett.DefaultAttr();
        sett.SetForSource(defAttr, rec.SrcName());

        DrawLineBG(startDrawPosition, defAttr.BG());

        CurrentRanges currRanges;
        RenderAttributes prevAttr = defAttr;

        QPoint drawPos = startDrawPosition;

        QString buffString;

        int lastCharBeyond = startRecOffs + recPosCount;

        for(int i = 0; i <= lastCharBeyond; ++i)
        {
            bool wasChanges = false;
            wasChanges |= currRanges.Leave(i);
            wasChanges |= currRanges.Enter(rec.PartsRanges(), i);
            
            if (wasChanges || (i == lastCharBeyond))
            {
                if (!buffString.isEmpty())                
                {
                    int chWidth = DrawString(drawPos, buffString, prevAttr);
                    drawPos.setX(drawPos.x() + chWidth);
                    buffString.clear();
                }

                prevAttr = defAttr;
                currRanges.Apply(sett.RangesFactory(), prevAttr);
            }

            if (i == lastCharBeyond) break;

            if (i >= startRecOffs) 
            {
                buffString += rec.Get().at(i);
            }

        }

    }



} // namespace UlvGui
