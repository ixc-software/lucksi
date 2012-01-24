
#include "stdafx.h"

#include "UdpLogFilter.h"


namespace
{
    const QString CSepAnd = "&&";
    const QString CSepOr = "||";

} // namespace

// -----------------------------------------------------------------

namespace Ulv
{
    bool UdpLogFilter::Match(const FilterExp &exp,
                             const QString &rec, int recStartPos,  bool recSrcSelected,
                             UdpLogRecordViewRangesList &addToRanges)
    {
        if (!recSrcSelected) return false;

        if (exp.IsEmpty()) return true;

        if (recStartPos == rec.size()) return false; // if data empty

        QString toCheck(rec.mid(recStartPos));
        if (toCheck.isEmpty()) return false;

        int filterPosOffset = recStartPos;
        Qt::CaseSensitivity cs = exp.CaseSensitiveQt();

        UdpLogRecordViewRangesList filterRanges;

        // or
        QStringList orList = exp.Get().split(CSepOr, QString::SkipEmptyParts,Qt::CaseSensitive);

        bool found = false;
        for (int i = 0; i < orList.size(); ++i)
        {
            // and
            QStringList andList = orList.at(i).split(CSepAnd, QString::SkipEmptyParts, Qt::CaseSensitive);

            found = FindAllOf(filterPosOffset, andList, cs, toCheck, filterRanges);
            if (found)
            {
                addToRanges.Add(filterRanges);
                return true;
            }
        }

        // not found -- delete all previously found filter ranges
        addToRanges.DropAllFilterRanges();

        return false;
    }

    // -----------------------------------------------------------------

    bool UdpLogFilter::FindAllOf(int filterPosOffset, // using in ranges
                                 const QStringList &expressions, Qt::CaseSensitivity cs,
                                 const QString &data,
                                 UdpLogRecordViewRangesList &addToFilterRanges)
    {
        if (expressions.isEmpty()) return true;

        for (int i = 0; i < expressions.size(); ++i)
        {
            QString toFind = expressions.at(i).trimmed();

            if (toFind.isEmpty()) continue;

            if (!FoundExp(filterPosOffset, toFind, cs, data, addToFilterRanges)) return false;
        }

        return true;
    }

    // -----------------------------------------------------------------

    bool UdpLogFilter::FoundExp(int filterPosOffset, // using in ranges
                                const QString &oneExp, Qt::CaseSensitivity cs,
                                const QString &data,
                                UdpLogRecordViewRangesList &addToFilterRanges)
    {
        ESS_ASSERT(!data.isEmpty());
        ESS_ASSERT(!oneExp.isEmpty());
        int filterRangesBefore = addToFilterRanges.Size();

        QStringList exceptExp = data.split(oneExp, QString::KeepEmptyParts, cs);

        // exp can be QRegExp and we can't predict it's size in string

        // int filterPosOffset = startPos;
        int filteredStartPos = (exceptExp.at(0).isEmpty())? 0 : exceptExp.at(0).size();
        int filteredSize = 0;

        // filter == 'aa'
        // data = 'aaabcdaaaaeaa'
        // --------------^----^ due to algorithm here filterSize == 0
        // as a result range of 'aaaa' == [6, 4]
        for (int i = 1; i  < exceptExp.size(); ++i)
        {
            // if filter at the end
            filteredSize = data.indexOf(exceptExp.at(i), filteredStartPos, cs) - filteredStartPos;
            if ((filteredSize == 0) && (i == (exceptExp.size() - 1)))
            {
                filteredSize = data.size() - filteredStartPos;
            }
            ESS_ASSERT(filteredSize >= 0);
            ESS_ASSERT((filteredStartPos + filteredSize) <= data.size());

            // save
            if (filteredSize != 0)
            {
                // add removable ranges
                addToFilterRanges.Add(filteredStartPos + filterPosOffset, filteredSize,
                                      UdpLogRecordViewRange::RTFilter);
            }

            // reset
            filteredStartPos += filteredSize;
            filteredStartPos += (exceptExp.at(i).isEmpty())? 0 : exceptExp.at(i).size();
            filteredSize = 0;
        }

        return (addToFilterRanges.Size() > filterRangesBefore);
    }

    // -----------------------------------------------------------------

} // namespace Ulv
