#ifndef __UTILS_OUTPUTFORMATTERS__
#define __UTILS_OUTPUTFORMATTERS__

namespace Utils
{

    class TableOutput
    {
        QString m_colSep;
        char m_charFiller;

        QVector<QStringList> m_table;
        QVector<int> m_userColWidthes;

        int MaxColumnWidth(int col) const;
        QVector<int> MakeRealColumnsWidthes() const;

        QStringList SplitCellToLines(const QString &text, int width) const;
        QStringList RowView(const QStringList &columns, const QVector<int> &widthes) const;

        void Add(const QString *pC1, const QString *pC2 = 0, const QString *pC3 = 0,
                 const QString *pC4 = 0, const QString *pC5 = 0, const QString *pC6 = 0);

    public:
        TableOutput(const QString &columnSep = "  ", char charFiller = ' ');

        QStringList Get() const;

        void AddRow(const QString &c1);
        void AddRow(const QString &c1, const QString &c2);
        void AddRow(const QString &c1, const QString &c2, const QString &c3);
        void AddRow(const QString &c1, const QString &c2, const QString &c3,
                    const QString &c4);
        void AddRow(const QString &c1, const QString &c2, const QString &c3,
                    const QString &c4, const QString &c5);
        void AddRow(const QString &c1, const QString &c2, const QString &c3,
                    const QString &c4, const QString &c5, const QString &c6);
        void AddRow(const QStringList &columns);

        // 0, -1 -- find max column width for this column
        void SetColumnWidthes(const QVector<int> &val);
    };

} // namespace Utils

#endif
