
#ifndef __DRIPARSERHELPERS__
#define __DRIPARSERHELPERS__


namespace DRI
{
    // Split input string 'what' by separator 'sep', avoid separators inside the double quotes.
    // Save result in QStringList 'res'.
    // Returns index of last found separator in string (if we need to split data
    // to full commands, it indicates was last command full or not)
    int SplitWithQuotesConsideration(const QString &what, QChar sep, 
                                     QStringList &res, bool skipEmpty = true);

} // namespace DRI

#endif

