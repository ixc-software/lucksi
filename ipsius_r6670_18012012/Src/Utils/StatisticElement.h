#ifndef STATISTICELEMENT_H
#define STATISTICELEMENT_H

#include "stdafx.h"
//#include "Utils/StatisticElementDetail.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/AddingWithOvrControl.h"
#include "Platform/PlatformTypes.h"

namespace Utils
{       

    namespace StatisticElementDetail
    {

        void FakeInit();

    }  // namespace StatisticElementDetail

    // Do statistic collection for type TElement
    template<class TElement, class TAccum, class TCounter = Platform::dword>
    class StatisticElement
    {       
        typedef StatisticElement<TElement, TAccum> TMy;
        //typedef Detail::IfFloating<TAccum> TIfFloating;

        TElement m_last;
        TElement m_min;
        TElement m_max;
        TAccum m_accum;
        TCounter m_count;
        bool m_overflow;    

        std::string m_prefix;
        std::string m_postfix;

        /*
        void Acummulate(const TAccum& data)
        {
            if (Utils::AddingWithOvrControl(m_accum, data)) m_overflowFlag = true;
        }

        void IncCount()
        {
            ++m_count;
            ESS_ASSERT(m_count != 0 && "Accum overflow");
        } */

        static bool Mask(int level, int mask)
        {
            return (level & mask) ? true : false;
        }

        void UpdateMinMax(const TElement &min, const TElement &max)
        {
            if (max > m_max) m_max = max;
            if (min < m_min) m_min = min;
        }

    public:

        StatisticElement(const std::string &prefix = "", const std::string &postfix = "")
            : m_prefix(prefix), m_postfix(postfix)
        {
            StatisticElementDetail::FakeInit();

            Reset();
        }
        
        TElement Average() const
        {
            ESS_ASSERT(!Empty());
            return static_cast<TElement>( m_accum / m_count );
        }

        const TElement& Max() const 
        {
            ESS_ASSERT(!Empty());
            return m_max;
        }

        const TElement& Min() const 
        {
            ESS_ASSERT(!Empty());
            return m_min;
        }

        const TElement& Last() const 
        {
            ESS_ASSERT(!Empty());
            return m_last;
        }

        TAccum Sum() const
        {
            ESS_ASSERT(!Empty());
            return m_accum;
        }

        int Count() const 
        {            
            return m_count;
        }

        bool Empty() const
        {
            return (m_count == 0);
        }

        void Add(const TElement& data)
        {   
            // first add
            if (m_count == 0) 
            {                
                Clear();

                m_max = data;
                m_min = data; 
            }

            // inc counter
            m_count++;
            if (m_count == 0)
            {
                m_overflow = true;
                m_accum = 0;
            }

            // accumulate
            if (Utils::AddingWithOvrControl(m_accum, data))
            {
                m_overflow = true;
            }

            // last, min, max
            UpdateMinMax(data, data);
            m_last = data;
        }              

        void Reset()
        {                        
            m_count = 0;
            m_accum = 0;
            m_overflow = false;   
        }

        void Clear()
        {
            Reset();
        }

        //TMy& operator +(const TMy& rhs)
        void Add(const TMy &rhs)
        {             
            if (rhs.Empty()) return;

            if (Empty())
            {
                *this = rhs;
                return;
            }

            UpdateMinMax(rhs.m_min, rhs.m_max);

            // accumulate
            if (Utils::AddingWithOvrControl(m_accum, rhs.m_accum))
            {
                m_overflow = true;
            }

            // counter
            if (Utils::AddingWithOvrControl(m_count, rhs.m_count))
            {
                *this = rhs;             // store just rhs
                m_overflow = true;   // bug indication
                return;
            }

            // overflow
            m_overflow = m_overflow || rhs.m_overflow;

            // last
            m_last = rhs.m_last;  // like Add() for single sample
        }

        void operator +=(const TMy& rhs)
        {
            Add(rhs);
        }

        enum FieldsMask
        {
            MaskLast        = 1 << 0,
            MaskAvg         = 1 << 1,
            MaskMin         = 1 << 2,
            MaskMax         = 1 << 3,
            MaskSum         = 1 << 4,
            MaskCount       = 1 << 5,
            MaskOverflow    = 1 << 6,
        };

        enum DetailLevel 
        {
            LevAverage          = MaskAvg | MaskOverflow,
            LevAvgLast          = MaskAvg | MaskLast | MaskOverflow,
            LevFull             = MaskLast | MaskAvg | MaskMin | MaskMax | MaskSum | MaskCount | MaskOverflow,
        };

        void ToString(std::string &s, int level = LevAverage) const
        {
            std::ostringstream ss;

            ss << m_prefix;

            if (Empty())
            {
                ss << "(empty)";
            }
            else
            {
                if (Mask(level, MaskLast))      ss << "last = "  << m_last << " ";
                if (Mask(level, MaskAvg))       ss << "avg = "   << (m_accum / m_count) << " ";
                if (Mask(level, MaskMin))       ss << "min = "   <<  m_min << " ";
                if (Mask(level, MaskMax))       ss << "max = "   <<  m_max << " ";
                if (Mask(level, MaskSum))       ss << "sum = "   <<  m_accum << " ";
                if (Mask(level, MaskCount))     ss << "count = " << m_count << " ";

                if (Mask(level, MaskOverflow) && m_overflow)  ss << "[!]";
            }

            ss << m_postfix;

            s = ss.str();
        }

        std::string ToString(int level = LevAverage) const
        {
            std::string s;

            ToString(s, level);

            return s;
        }
		template<class Data, class TStream>
			static void Serialize(Data &data, TStream &s)
		{
			s
				<< data.m_last
				<< data.m_min
				<< data.m_max
				<< data.m_accum
				<< data.m_count
				<< data.m_overflow;
		}
        
    };

    // must be sync'ed with .cpp file template instance
    typedef StatisticElement<int, Platform::int64> StatElementForInt;

    /*
    typedef StatisticElement<Platform::byte, Platform::word> StatElementForUByte;
    typedef StatisticElement<char, int> StatElementForSByte;
    typedef StatisticElement<Platform::ddword, Platform::ddword> StatElementForDDWord;
    typedef StatisticElement<Platform::dword, Platform::ddword> StatElementForDWord; */
   
} // namespace Utils

#endif
