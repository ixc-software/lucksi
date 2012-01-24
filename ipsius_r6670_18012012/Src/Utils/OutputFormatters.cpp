
#include "stdafx.h"
#include "OutputFormatters.h"
#include "Utils/ErrorsSubsystem.h"


namespace Utils
{

    // ---------------------------------------------------------------------------
    // TableOutput impl

    TableOutput::TableOutput(const QString &columnSep, char charFiller) :
        m_colSep(columnSep), m_charFiller(charFiller)
    {        
    }

    // ---------------------------------------------------------------------------

    QStringList TableOutput::SplitCellToLines(const QString &text, int width) const
    {
        ESS_ASSERT(width >= 0);
        QStringList res;
        if (text.isEmpty())
        {
            res.push_back(QString(width, m_charFiller));
            return res;
        }

        int i = 0;
        while (i < text.size())
        {
            QString line = text.mid(i, width);
            if (line.size() < width) line += QString(width - line.size(), m_charFiller);
            res.append(line);
            i += width;
        }

        return res;
    }

    // ---------------------------------------------------------------------------

    int TableOutput::MaxColumnWidth(int col) const
    {
        int max = 0;
        for (int row = 0; row < m_table.size(); ++row)
        {
            int colCount = m_table.at(row).size();
            if (col >= colCount) continue;

            int currWidth = m_table.at(row).at(col).size();
            if (max < currWidth) max = currWidth;
        }

        return max;
    }

    // ---------------------------------------------------------------------------

    QVector<int> TableOutput::MakeRealColumnsWidthes() const
    {
        QVector<int> res;
        for (int i = 0; i < m_userColWidthes.size(); ++i)
        {
            int max = (m_userColWidthes.at(i) > 0)? m_userColWidthes.at(i) : MaxColumnWidth(i);

            res.push_back(max);
        }

        return res;
    }

    // ---------------------------------------------------------------------------

    QStringList TableOutput::RowView(const QStringList &columns, const QVector<int> &widthes) const
    {
        // cells in multi lines
        QVector<QStringList> rowTable;
        int maxRowCount = 0;
        for (int i = 0; i < widthes.size(); ++i)
        {
            QString column = (i < columns.size()) ? columns.at(i) : "";
            
            QStringList cellLines(SplitCellToLines(column, widthes.at(i)));
            if (cellLines.size() > maxRowCount) maxRowCount = cellLines.size();

            rowTable.push_back(cellLines);
        }

        // make empty row lines
        QStringList res;
        for (int i = 0; i < maxRowCount; ++i)
        {
            res.push_back("");
        }

        // fill row lines
        for (int iCol = 0; iCol < rowTable.size(); ++iCol)
        {
            QStringList col = rowTable[iCol];

            // add cells lines to row
            for (int iRow = 0; iRow < col.size(); ++iRow)
            {
                if (iCol != 0) res[iRow] += m_colSep;
                res[iRow] += col[iRow];
            }

            // add empty rows inside cell
            for (int iRow = col.size(); iRow < maxRowCount; ++iRow)
            {
                if (iCol != 0) res[iRow] += m_colSep;
                res[iRow] += QString(widthes.at(iCol), m_charFiller);
            }            
        }

        return res;
    }

    // ---------------------------------------------------------------------------

    void TableOutput::Add(const QString *pC1, const QString *pC2, const QString *pC3,
                          const QString *pC4, const QString *pC5, const QString *pC6)
    {
        QStringList row;

        ESS_ASSERT(pC1 != 0);
        row.push_back(*pC1);

        if (pC2 != 0) row.push_back(*pC2);
        if (pC3 != 0) row.push_back(*pC3);
        if (pC4 != 0) row.push_back(*pC4);
        if (pC5 != 0) row.push_back(*pC5);
        if (pC6 != 0) row.push_back(*pC6);

        AddRow(row);
    }

    // ---------------------------------------------------------------------------

    void TableOutput::AddRow(const QStringList &columns)
    {
        for (int i = m_userColWidthes.size(); i < columns.size(); ++i)
        {
            m_userColWidthes.push_back(-1);
        }

        m_table.push_back(columns);
    }

    // ---------------------------------------------------------------------------

    void TableOutput::AddRow(const QString &c1)
    {
        Add(&c1);
    }

    // ---------------------------------------------------------------------------

    void TableOutput::AddRow(const QString &c1, const QString &c2)
    {
        Add(&c1, &c2);
    }

    // ---------------------------------------------------------------------------

    void TableOutput::AddRow(const QString &c1, const QString &c2, const QString &c3)
    {
        Add(&c1, &c2, &c3);
    }

    // ---------------------------------------------------------------------------

    void TableOutput::AddRow(const QString &c1, const QString &c2, const QString &c3,
                             const QString &c4)
    {
        Add(&c1, &c2, &c3, &c4);
    }

    // ---------------------------------------------------------------------------

    void TableOutput::AddRow(const QString &c1, const QString &c2, const QString &c3,
                             const QString &c4, const QString &c5)
    {
        Add(&c1, &c2, &c3, &c4, &c5);
    }

    // ---------------------------------------------------------------------------

    void TableOutput::AddRow(const QString &c1, const QString &c2, const QString &c3,
                             const QString &c4, const QString &c5, const QString &c6)
    {
        Add(&c1, &c2, &c3, &c4, &c5, &c6);
    }

    // ---------------------------------------------------------------------------

    void TableOutput::SetColumnWidthes(const QVector<int> &val)
    {
        int max = m_userColWidthes.size();

        ESS_ASSERT(val.size() <= max);
        for (int i = 0; i < max; ++i)
        {
            int width =  (i < val.size())? val.at(i) : -1;

            m_userColWidthes[i] = width;
        }
    }

    // ---------------------------------------------------------------------------

    QStringList TableOutput::Get() const
    {
        QStringList res;

        QVector<int> widthes(MakeRealColumnsWidthes());

        for (int i = 0; i < m_table.size(); ++i)
        {
            res += RowView(m_table.at(i), widthes);
        }

        return res;
    }

    // ---------------------------------------------------------------------------

} // namespace Utils
