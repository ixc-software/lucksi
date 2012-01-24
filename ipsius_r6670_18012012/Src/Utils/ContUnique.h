#pragma once

#include "ComparableT.h"

namespace Utils
{

    template<class TCont, class TCmpFn>
    bool AllItemsUnique(const TCont &c, TCmpFn cmpFn)
    {
        typename TCont::const_iterator i = c.begin();
        while(i != c.end())
        {
            typename TCont::const_iterator iTail = i + 1;
            while(iTail != c.end())
            {
                if ( cmpFn(*i, *iTail) == 0 )  return false;
                ++iTail;
            }
                
            ++i;
        }

        return true;
    }

    template<class TCont>
    bool AllItemsUnique(const TCont &c)
    {
        return AllItemsUnique(c, CompareToInt<typename TCont::value_type>);
    }
           
    
}  // namespace Utils
