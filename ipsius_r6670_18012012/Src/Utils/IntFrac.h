#ifndef __INTFRAC__
#define __INTFRAC__

#include "Platform/PlatformTypes.h"
#include "Utils/ComparableT.h"

namespace Utils
{
	
    /*
        Класс, содержащий целое число и дробную часть разрядности BitsNumber 
        Пока что работает только для положительных чисел, сохраняемое значение
        можно без всяких корректив складывать и вычитать
    */
    template<int BitsNumber, class TStoreType = Platform::dword>	
    class IntFrac : 
        public Utils::ComparableT< IntFrac<BitsNumber, TStoreType> >
    {
        typedef IntFrac<BitsNumber, TStoreType> TSelf;

        enum
        {
            CFracRange      = (1 << BitsNumber),
            CFracRangeHalf  = CFracRange / 2,
            CFracBitMask    = (CFracRange - 1),
        };

        TStoreType m_value;

        static TStoreType Make(TStoreType intPart)
        {
            TStoreType res;

            res = intPart << (TStoreType)BitsNumber;

            return res;
        }

        static TStoreType Make(TStoreType intPart, double fracPart)
        {
            TStoreType res = Make(intPart);  // int part

            // frac part to int
            res |= (TStoreType)(fracPart * CFracRange);

            return res;
        }

    public:

        IntFrac()
        {
            m_value = 0;
        }

        explicit IntFrac(double f)
        {
            double intPart;
            double fracPart = std::modf(f, &intPart);

            m_value = Make( (TStoreType)intPart, fracPart );            
        }

        explicit IntFrac(int val)
        {
            m_value = Make(val);
        }

        void operator=(int val)
        {
            m_value = Make(val);
        }

        int Compare(const TSelf &other) const
        {
            if (m_value == other.m_value) return 0;

            return (m_value > other.m_value) ? 1 : -1;
        }

        TStoreType IntPart() const
        {
            return m_value >> (TStoreType)BitsNumber;
        }

        TStoreType FracPart() const
        {
            return m_value & CFracBitMask;
        }

        TStoreType IntPartRounded() const
        {
            TStoreType val = IntPart();

            if (FracPart() > CFracRangeHalf) ++val;

            return val;
        }

        void operator+=(const TSelf &other)
        {
            m_value += other.m_value;
        }

        void operator-=(const TSelf &other)
        {
            m_value -= other.m_value;
        }

        std::string ToString() const
        {
            std::ostringstream ss;
            ss << IntPart() << "/" << FracPart();
            return ss.str();
        }

    };
	
}  // namespace Utils

#endif
