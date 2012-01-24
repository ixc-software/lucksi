#ifndef __ULVGUIIMAGERENDER__
#define __ULVGUIIMAGERENDER__

#include "Core/UdpLogRecordViewRange.h"
#include "Core/UdpLogRecord.h"

#include "UlvGuiCtrlToModelParams.h"


namespace UlvGui
{
    class RangeGuiSett;

    // Currenr settings for image renderer
    class RenderAttributes
    {
        // gui params
        QFont m_font;
        QBrush m_bg;
        QPen m_pen;

    public:
        RenderAttributes() // empty
        {}

        RenderAttributes(const QString &fontName, size_t fontSize, const QColor &bg) : // with default parameters
                m_font(fontName, fontSize), m_bg(bg)
        {
        }

        void FontName(const QString &val) { m_font.setFamily(val); }
        void FontSize(int val)
        {
            ESS_ASSERT(val > 0);
            m_font.setPointSize(val);
        }
        void FontColor(const QColor &val) { m_pen.setColor(val); }
        void FontBGColor(const QColor &val) { m_bg.setColor(val); }

        const QFont& Font() const { return  m_font; }
        const QBrush& BG() const { return  m_bg; }
        const QPen& Pen() const { return  m_pen; }
    };

    // ------------------------------------------------------------

    // UdpLorRecordView settings
    class RangeGuiSett
    {
        Ulv::UdpLogRecordViewRange::Type m_type;

        // QString m_fontName;
        // int m_fontSize;

        bool m_useFontColor;
        QColor m_fontColor;

        bool m_useFontBGColor;
        QColor m_fontBGColor;

    public:
        // example: RTSenderTimestamp: color=white bold=true
        RangeGuiSett(Ulv::UdpLogRecordViewRange::Type type, const QString &sett);
        RangeGuiSett(Ulv::UdpLogRecordViewRange::Type type,
                     const QColor &fontColor, const QColor &fontBGColor);

        void Apply(RenderAttributes &attr, Ulv::UdpLogRecordViewRange::Type type) const;
    };

    // ------------------------------------------------------------

    // RangeGuiSett factory
    class RangeGuiSettFactory
    {
        std::vector<RangeGuiSett> m_list;

    public:
        RangeGuiSettFactory();

        // Fing GUI sett by type and apply them to attr
        void Apply(RenderAttributes &attr, Ulv::UdpLogRecordViewRange::Type type) const;
    };


    // ------------------------------------------------------------

    // All draw settings
    class RenderGuiSettings
    {
        RenderAttributes m_default;
        UlvGui::RecordSrcGuiSettList m_recSrcSettList;
        RangeGuiSettFactory m_rangesFactory;
        bool m_ignoreRecLF;


    public:
        RenderGuiSettings(const GuiCtrlToModelParams &sett);

        const RenderAttributes& DefaultAttr() const { return m_default; }

        // set background depend on src
        void SetForSource(RenderAttributes &attr, const QString &srcName) const;

        const RangeGuiSettFactory& RangesFactory() const { return m_rangesFactory; }
        bool IgnoreLogRecordLF() const { return m_ignoreRecLF; }
    };

    // ------------------------------------------------------------

    // Using to draw log DB record onto image
    class ImageRender
    {
        boost::shared_ptr<QImage> m_image;
        QPainter m_painter;
        RenderGuiSettings m_sett;

        int LineCharCount(int startX) const;
        void RecordLines(const Ulv::UdpLogRecordView &rec,
                         std::vector<int> &linesSizes) const;
        void AppendRecordLines(int textSize, std::vector<int> &linesSizes) const;

        void DrawBG(const QRect &rect, const QBrush &bg, int borderSize = 0);
        void DrawLineBG(const QPoint &topLeft, const QBrush &bg);

        void Draw(const QPoint &startDrawPosition,
                  const RenderGuiSettings &sett,
                  const Ulv::UdpLogRecordView &rec,
                  int startRecOffs, int recPosCount);

        int DrawString(const QPoint &drawPos, const QString &s,
                       const RenderAttributes &currAttr);

    public:
        ImageRender(const GuiCtrlToModelParams &sett);
        boost::shared_ptr<QImage> get() const { return m_image; }

        int MaxLinesCount() const;
        int LineHeight() const;

        int RecordLinesCount(const Ulv::UdpLogRecordView &rec) const;

        // returns number of lines rendered
        // direction > 0 -- draw from top to bottom
        // direction < 0 -- draw from bottom to top
        int DrawOneRecord(int lineTopY, const Ulv::UdpLogRecordView &rec, int direction);
    };

} // namespace UlvGui

#endif
