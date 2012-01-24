
#include "stdafx.h"
#include "DRIParserHelpers.h"
#include "ParsedDRICmd.h"
#include "DRIParserConsts.h"

namespace
{
    void AddTrimmed(QStringList &to, const QString &what, bool skipEmpty)
    {
        QString tmp( what.trimmed() );
        if ( ( tmp.isEmpty() ) && (skipEmpty) ) return;

        to << tmp;
    }

} // namespace 

// -------------------------------------------------------------

namespace DRI
{
    int SplitWithQuotesConsideration(const QString &what, QChar sep, 
                                     QStringList &res, bool skipEmpty)
    {
        int lastSepIndex = -1;
        res.clear(); // reset data

        // if no separator
        if (what.indexOf(sep) < 0) 
        {
            AddTrimmed(res, what, skipEmpty);
            return lastSepIndex;
        }

        // split
        int quoteCount = 0;
        for (int i = 0; i < what.size(); ++i)
        {
            if (what.at(i) == CStringChar) ++quoteCount;

            if ( (what.at(i) == sep) && ( (quoteCount % 2) == 0 ) )
            {
                int begin = lastSepIndex + 1;
                AddTrimmed(res, what.mid( begin, (i - begin) ), skipEmpty);
                lastSepIndex = i;
            }
        }

        if ( lastSepIndex == (what.size() - 1) ) return lastSepIndex;

        // add data after last separator
        AddTrimmed(res, what.mid(lastSepIndex + 1), skipEmpty);

        return lastSepIndex;
    }

} // namespace DRI

