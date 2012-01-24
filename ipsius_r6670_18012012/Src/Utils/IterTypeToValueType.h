#ifndef ITERTYPETOVALUETYPE_H
#define ITERTYPETOVALUETYPE_H

#include "stdafx.h"

namespace Utils
{
    namespace mpl = boost::mpl;

    template<class TIter>
    class IterTypeToValueType
    {
        struct TrivialIterToValue
        {
            template<class TIterator>
            struct Solve
            {
                typedef typename TIterator::value_type TValue;                
            };
        };

        struct InsertIterToValue
        {
            template<class TIterator>
            struct Solve
            {
                typedef typename TIterator::container_type::value_type TValue;
            };
        };

        struct TrueIterator
        {
            template<class TIterator>
            struct getValType
            {     
                /*
                typedef typename mpl::if_<
                typename boost::is_void<typename TIter::value_type>::type, 
                InsertIterToValue, 
                TrivialIterToValue
                >::type TIterValue;

                typedef typename TIterValue::Solve<TIter>::TValue TValue;
                */

                typedef typename mpl::if_<
                    typename boost::is_void<typename TIter::value_type>::type, 
                    InsertIterToValue, 
                    TrivialIterToValue
                >::type TIterValue;

                typedef typename TIterValue::template Solve<TIter>::TValue TValue;

            };
        };

        struct PointerIterator        
        {
            template<class TIterator>
            struct getValType
            {
                typedef typename boost::remove_pointer<TIterator>::type TValue;
            };
        };

        typedef typename mpl::if_<
            typename boost::is_pointer<TIter>::type,
            PointerIterator,
            TrueIterator
        >::type TypeOfIter;

    public:

        typedef typename TypeOfIter::template getValType<TIter>::TValue TValue;

    };
} // namespace Utils

#endif
