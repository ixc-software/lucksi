
#include "stdafx.h"

#include "UlvGuiCtrlToModelParams.h"


namespace UlvGui
{
    // -------------------------------------------------------------------
    // RecordSrcGuiSettList impl

    RecordSrcGuiSettList::Item::Item(const QString &srcName, const QColor &bg) :
        SrcName(srcName), BGColor(bg)
    {
    }

    // -------------------------------------------------------------------

    int RecordSrcGuiSettList::Find(const QString &name) const
    {
        for (size_t i = 0; i < m_list.size(); ++i)
        {
            QString curr = m_list.at(i).SrcName;
            if (curr == name) return i;
        }

        return -1;
    }

    // -------------------------------------------------------------------

    void RecordSrcGuiSettList::Add(const QString &srcName, const QColor &bg)
    {
        ESS_ASSERT(Find(srcName) < 0);
        m_list.push_back(Item(srcName, bg));
    }

    // -------------------------------------------------------------------

    QColor RecordSrcGuiSettList::GetColor(const QString &srcName) const
    {
        ESS_ASSERT(!m_list.empty());

        int index = Find(srcName);

        if (index < 0) return QColor(Qt::white);

        return m_list.at(index).BGColor;
    }

    // -------------------------------------------------------------------
    // GuiCtrlToModelParams impl

    void GuiCtrlToModelParams::SrcSett(const RecordSrcGuiSettList &sett)
    {
        m_recSrcSettList = sett;
    }

    // ---------------------------------------------------------------------------------

    RecordSrcGuiSettList GuiCtrlToModelParams::SrcSett() const
    {
        return m_recSrcSettList;
    }

    // ---------------------------------------------------------------------------------

    void GuiCtrlToModelParams::LogWindow(int width, int height)
    {
        ESS_ASSERT((width > 0) && (height > 0));
        m_logWindowWidth = width;
        m_logWindowHeight = height;
    }

    // ---------------------------------------------------------------------------------

    void GuiCtrlToModelParams::LogFont(const QString &fontName, int fontSize)
    {
        ESS_ASSERT(!fontName.isEmpty());
        ESS_ASSERT(fontSize > 0);

        m_logFontName = fontName;
        m_logFontSize = fontSize;
    }

} // namespace UlvGui
