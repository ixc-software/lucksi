#ifndef __ULVGUICTRLTOMODELPARAMS__
#define __ULVGUICTRLTOMODELPARAMS__

#include "Utils/ManagedList.h"
#include "Core/CtrlToModelParams.h"

namespace UlvGui
{
    class RecordSrcGuiSettList
    {
        struct Item
        {
            QString SrcName;
            QColor BGColor;
            
            Item(const QString &srcName, const QColor &bg);
        };

        std::vector<Item> m_list;

        int Find(const QString &name) const;

    public:
        RecordSrcGuiSettList()
        {}

        void Add(const QString &srcName, const QColor &bg);

        // for unknown names return default color
        QColor GetColor(const QString &srcName) const;

        int Size() const { return m_list.size(); }
    };

    // ------------------------------------------------------------

    class GuiCtrlToModelParams :
        public Ulv::CtrlToModelParams
    {
        int m_logWindowWidth;
        int m_logWindowHeight;

        QString m_logFontName;
        int m_logFontSize;

        bool m_ignoreLogRecLF;

        RecordSrcGuiSettList m_recSrcSettList;

    public:
        GuiCtrlToModelParams() :
            m_logWindowWidth(0), m_logWindowHeight(0),
            m_logFontSize(0), m_ignoreLogRecLF(false)
        {
        }

        // store all src settings
        void SrcSett(const RecordSrcGuiSettList &sett);
        RecordSrcGuiSettList SrcSett() const;

        void LogWindow(int width, int height);
        int LogWindowWidth() const { return m_logWindowWidth; }
        int LogWindowHeight() const { return m_logWindowHeight; }

        void LogFont(const QString &fontName, int fontSize);
        const QString& LogFontName() const { return m_logFontName; }
        int LogFontSize() const { return m_logFontSize; }

        void IgnoreLogRecordLF(bool val) { m_ignoreLogRecLF = val; }
        bool IgnoreLogRecordLF() const { return m_ignoreLogRecLF; }
    };

} // namespace UlvGui

#endif
