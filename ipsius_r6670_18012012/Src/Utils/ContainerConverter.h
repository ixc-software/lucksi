#ifndef CONTAINERCONVERTER_H
#define CONTAINERCONVERTER_H

#include "stdafx.h"
#include "IterTypeToValueType.h"

namespace Utils
{
    namespace mpl = boost::mpl;

    // Функторы-преобразователи данных для  Converter    
    template<class TOut>
    class NoCastConvert
    {
    public:
        template<class TIn>
        TOut operator()(const TIn& src)
        {
            return src;
        }
    };

    template<class TOut>
    class StaticCastConvert
    {
    public:
        template<class TIn>
        TOut operator()(const TIn& src)
        {
            return static_cast<TOut>(src);
        }
    };

    template<class TOut>
    class ReinterpretCastConvert
    {
    public:
        template<class TIn>
        TOut operator()(const TIn& src)
        {
            return reinterpret_cast<TIn>(src);
        }
    };

    template<class TOut>
    class LexicalCastConvert
    {
    public:
        template<class TIn>
        TOut operator()(const TIn& src)
        {
            return boost::lexical_cast<TIn>(src);
        }
    };

    // Преобразователь контейнерных данных. 
    // Параметризуется функтором-преобразователем данных.
    template<template<class> class TDataConverter = NoCastConvert >
    class Converter
    {      
        static const bool CUseedMpl = false; // использование альтернативного решения с mpl
        
        template<class TContainer>        
        static void CheckedClear(TContainer& cont)
        {
            if (!cont.empty()) cont.clear();            
        }

        template<class TContainer>        
        static void ClearThruSwap(TContainer &cont)
        {   
            // if (!cont.empty()) cont.swap(TContainer());

            TContainer empty;
            if (!cont.empty()) cont.swap(empty);
        }
        
        template<class TArg>        
        static void CheckedClear(std::vector<TArg>& cont)
        {
            ClearThruSwap(cont);
        }
        
        template<class TArg>        
        static void CheckedClear(std::deque<TArg>& cont)
        {
            ClearThruSwap(cont);
        }        

        // перегрузка для нешаблонного итератора
        template<class TIter>
        static TDataConverter<typename TIter::value_type> DeduceConverterType(TIter)
        {
            return TDataConverter<typename TIter::value_type>();
        }

        // перегрузка для шаблонного итератора
        template<class TCont, template<class> class TIter> // выыкдение шаблонного параметра шаблонного шаблонного типа
        static TDataConverter<typename TCont::value_type> DeduceConverterType(TIter<TCont>)
        {
            return TDataConverter<typename TCont::value_type>();
        }        

        // перегрузка для нешаблонного итератора и специализация для указателя
        template<class TData>
        static TDataConverter<TData> DeduceConverterType(TData*)
        {
            return TDataConverter<TData>();
        }

    public:
        
        // warning -- iterOutput must be inserter iterator in most use cases
        template<class TInIterator, class TOutIterator>
        static void DoConvert(TInIterator from, TInIterator to, TOutIterator iterOutput)
        {   
            if (CUseedMpl)
            {                
                typedef typename IterTypeToValueType<TOutIterator>::TValue TValue;

                TDataConverter<TValue> converter;            

                std::transform(from, to, iterOutput, converter);
            }
            else

                std::transform(from, to, iterOutput, DeduceConverterType(iterOutput));
        }

        
        template<class TInContainer, class TOutContainer>
        static void DoConvert(const TInContainer src, TOutContainer& dst, bool clearDst = true)
        {            
            if (clearDst) CheckedClear(dst);            
            DoConvert( src.begin(), src.end(), std::back_inserter(dst) );
        }
    };    
       
    void TestConverter();

} // namespace Utils

// TODO: check value not pointer

#endif
