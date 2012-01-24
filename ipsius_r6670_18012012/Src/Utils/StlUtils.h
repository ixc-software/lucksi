#pragma once

namespace Utils
{
    
    template<class TContDst, class TContSrc>    
    void ContAppend(TContDst &dst, const TContSrc &src)
    {
        typename TContSrc::const_iterator i = src.begin();

        while( i != src.end() )
        {
            dst.push_back( *i );
            ++i;
        }
    }

    template<class TContDst, class TContSrc>    
    void ContAppendSame(TContDst &dst, const TContSrc &src)
    {
        const bool _Check = boost::is_same<typename TContDst::value_type, typename TContSrc::value_type>::value;
        BOOST_STATIC_ASSERT( _Check );
        ContAppend(dst, src);
    }
           
}  // namespace Utils
