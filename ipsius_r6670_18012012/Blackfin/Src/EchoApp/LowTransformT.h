#ifndef __LOWTRANSFORMT__
#define __LOWTRANSFORMT__

#include "Platform/PlatformTypes.h"
#include "Utils/IBasicInterface.h"
#include "iDSP/g711codec.h"
#include "Utils/Random.h"
#include "Utils/RawCRC32.h"
#include "Utils/GlobalMutex.h"

namespace EchoApp
{
    namespace mpl = boost::mpl; 

    using Platform::byte;
    using Platform::int16;
    using Platform::word;
    using Platform::dword;

    // -------------------------------------------------------------------

    /*
    template<bool TExpand>
    struct LowTypes
    {
        typedef typename mpl::if_<mpl::bool_<TExpand>, byte, int16>::type TIn;
        typedef typename mpl::if_<mpl::bool_<TExpand>, int16, byte>::type TOut;
    }; */

    template<bool TExpand>
    struct LowTypes
    {
        typedef byte TIn;
        typedef int16 TOut;
    };

    template<>
    struct LowTypes<false>
    {
        typedef int16 TIn;
        typedef byte TOut;
    };

    // -------------------------------------------------------------------

    template<int TMin, int TMax, class TLow>
    class LowBase
    {
        static const int CRange = (-TMin) + TMax + 1;

        BOOST_STATIC_ASSERT(TMin < 0);
        BOOST_STATIC_ASSERT(TMax > 0);

    public:

        // fill table for compess
        static void FillTable(std::vector<byte> &t)
        {
            const int CSize = 0x10000;  // unsigned, map as signed into [TMin .. TMax]
            t.clear();
            t.reserve(CSize);

            for(int i = 0; i < CSize; ++i)
            {
                int16 val = (word)i;
                if (val > TMax) val = TMax;
                if (val < TMin) val = TMin;
                t.push_back( TLow::Compress(val) );
            }
        }

        // fill table for extract
        static void FillTable(std::vector<int16> &t)
        {
            const int CSize = 0x100;
            t.clear();
            t.reserve(CSize);

            for(int i = 0; i < CSize; ++i)
            {
                t.push_back( TLow::Expand(i) );
            }
        }

        static int16 NextLinear(Utils::Random &rnd)
        {
            int16 val = rnd.Next(CRange) + TMin;
            ESS_ASSERT( (val >= TMin) && (val <= TMax));
            return val;
        }

        static std::string Name(bool table, bool expand)
        {
            std::ostringstream oss;

            oss << TLow::LowName() << (expand ? " expand  " : " compress")
                << (table ? " table" : " math ");

            return oss.str();
        }

    };

    // ----------------------------------------------------------

    struct ALowBase : public LowBase<iDSP::CALowMin, iDSP::CALowMax, ALowBase>
    {
        static byte Compress(int16 val)
        {
            return iDSP::linear2alaw(val);
        }

        static void Compress(const int16 *in, byte *out, int count)
        {
            while(count--)
            {
                *out++ = Compress(*in++);
            }
        }

        static int16 Expand(byte val)
        {
            return iDSP::alaw2linear(val);
        }

        static void Expand(const byte *in, int16 *out, int count)
        {
            while(count--)
            {
                *out++ = Expand(*in++);
            }
        }

        static const char* LowName()
        {
            return "A-low";
        }

    };


    struct ULowBase : public LowBase<iDSP::CULowMin, iDSP::CULowMax, ULowBase>
    {
        static byte Compress(int16 val)
        {
            return iDSP::linear2ulaw(val);
        }

        static void Compress(const int16 *in, byte *out, int count)
        {
            while(count--)
            {
                *out++ = Compress(*in++);
            }
        }

        static int16 Expand(byte val)
        {
            return iDSP::ulaw2linear(val);
        }

        static void Expand(const byte *in, int16 *out, int count)
        {
            while(count--)
            {
                *out++ = Expand(*in++);
            }
        }

        static const char* LowName()
        {
            return "U-low";
        }

    };

    // ----------------------------------------------------------

    template<bool TExpand, class TLow>
    struct LowTemplateMath : public TLow
    {
        typedef typename LowTypes<TExpand>::TIn TIn;
        typedef typename LowTypes<TExpand>::TOut TOut;

        static const bool CExpand = TExpand;

        static void Init()
        {
            // nothing
        }

        static TOut Process(TIn val)
        {
            if (TExpand) return TLow::Expand(val);
                    else return TLow::Compress(val);
        }

        static void Process(const TIn *in, TOut *out, int count)
        {
            // Action<TExpand>::Do(in, out, count);
            BlockProcess<TLow>(in, out, count);
        }

        static std::string Name()
        {
            return TLow::Name(false, TExpand);
        }

        static bool TableCRC(dword &val)
        {
            return false;
        }

    private:

        template<class T>
        static void BlockProcess(const int16 *in, byte *out, int count)
        {
            T::Compress(in, out, count);
        }

        template<class T>
        static void BlockProcess(const byte *in, int16 *out, int count)
        {
            T::Expand(in, out, count);
        }

    };
    
    // specialization for table
    template<bool TExpand, class TLow>
    struct LowTemplateTable : public TLow
    {
        typedef typename LowTypes<TExpand>::TIn TIn;
        typedef typename LowTypes<TExpand>::TOut TOut;

        static const bool CExpand = TExpand;

        static void Init()
        {
            if (PTable != 0) return;
            PTable = &(Vector()[0]);
        }

        // you must call Init() before!
        static TOut Process(TIn val)
        {
            // ESS_ASSERT(PTable != 0);  // hard optimisation -- it's dangerous!
            return PTable[(word)val];
        }

        static void Process(const TIn *in, TOut *out, int count)
        {
            ESS_ASSERT(PTable != 0);
            const TOut *pT = PTable;  // Table()

            while(count--)
            {
                TIn val = *in++;
                *out++ = pT[(word)val];
            }
        }

        static std::string Name()
        {
            return TLow::Name(true, TExpand);
        }

        static bool IsTable()
        {
            return true;
        }

        static bool TableCRC(dword &val)
        {
            Init();
            val = Utils::UpdateCRC32(Vector());
            return true;
        }

    private:

        static const TOut* PTable;

        static const std::vector<TOut>& Vector()
        {
            Utils::GlobalMutexLocker locker;

            {
                static std::vector<TOut> t;
                if (t.empty()) TLow::FillTable(t);
                return t;
            }
        }

        /*
        static const TOut* Table()  // lazy init, it's slow for single sample path
        {
            static std::vector<TOut> t;

            if (t.empty())
            {
                TLow::FillTable(t);
            }

            return &(t[0]);
        }  */

    };

//    template<bool TExpand, class TLow>
//    const typename LowTypes<TExpand>::TOut* LowTemplateTable<TExpand, TLow>::PTable = 0; //doesnt compile by gcc

    template<bool TExpand, class TLow>
    const typename LowTemplateTable<TExpand, TLow>::TOut* LowTemplateTable<TExpand, TLow>::PTable = 0;

}  // namespace EchoApp

#endif

