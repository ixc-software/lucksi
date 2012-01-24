
#include "stdafx.h"
#include "StatisticBaseTest.h"
#include "Utils/StatisticBase.h"

// Test classes
namespace 
{
    using namespace Platform;
    using namespace Utils;

    struct StatTest : public StatisticBase<StatTest>
    {
        dword Field;
        int Field2;
        float Field3;
        StatElementForInt Stat1;
        
        StatTest() : StatisticBase<StatTest>()
        {
            Clear();
        }

        STATISTIC_META_INFO(StatTest)
        {
            meta.Add(inst.Field, "Field");
            meta.Add(inst.Field2, "Field2");
            meta.Add(inst.Field3, "Field3");
            meta.Add(inst.Stat1, "Stat1", StatElementForInt::LevAvgLast);
        }

        size_t FildsCount() { return Meta().Count(); }
    }; 

    // -----------------------------------------------------------

    struct StatTestOther : public StatisticBase<StatTestOther>
    {
        int Field;
        int Field2;
        
    public:
        StatTestOther() : StatisticBase<StatTestOther>()
        {
            Clear();
        }

        STATISTIC_META_INFO(StatTestOther)
        {
            meta.Add(inst.Field, "Field");
            meta.Add(inst.Field2, "Field2");
        }

        void ToString(std::string &res, const std::string &sep, bool showFieldType) const // override
        {
            res.clear();
            for (size_t i = 0; i < Meta().Count(); ++i)
            {
                StatTestOther *p = const_cast<StatTestOther*>(this);  // lame, just for shut up compiler
                res += FieldToString(Meta()[i].ToString(*p, showFieldType));
                if (i < (Meta().Count() - 1)) res += sep;
            }
        }

        std::string ToString(const std::string &sep, bool showFieldType) const
        {
            return TBase::ToString(sep, showFieldType);
        }

        static std::string FieldToString(const std::string &field)
        {
            std::string res;
            res += "<prefix>";
            res += field;
            res += "<postfix>";

            return res;
        }
    };

} // namespace 

/*
'Utils::FieldBase<int, <unnamed>::StatTestOther>::FieldToString(const Utils::MetaField<int, <unnamed>::StatTestOther>&, <unnamed>::StatTestOther&, bool&, std::basic_string<char, std::char_traits<char>, std::allocator<char> >&) const'
                Utils::FieldBase<TField, TOwner>::FieldToString(Utils::MetaField<TField, TOwner>&, TOwner&, bool, std::string&) const [with TField = int, TOwner = <unnamed>::StatTestOther]
*/


// -----------------------------------------------------------
// Tests helpers
namespace 
{
    using namespace Utils;
    
    void Check(StatTest &test, dword f, int f2, float f3, int s1)
    {
        TUT_ASSERT(test.Field == f);
        TUT_ASSERT(test.Field2 == f2);
        TUT_ASSERT(test.Field3 == f3);
        if (!test.Stat1.Empty()) TUT_ASSERT(test.Stat1.Average() == s1);
        else if (s1 != 0) TUT_ASSERT(0 && "test.Stat1.Empty()"); 
    }

    // -----------------------------------------------------------

    std::string FieldToStr(const std::string &name, int val)
    {
        std::ostringstream os;
        os << name << MetaFieldDetail::CMetaFieldAndValueSep << val;

        return os.str();
    }

    // -----------------------------------------------------------
    
    void CheckString(StatTestOther &stat, const std::string &sep, bool silentMode)
    {
        std::string str = stat.ToString(sep, false);

        std::ostringstream check;

        check << StatTestOther::FieldToString(FieldToStr("Field", stat.Field)) << sep
            <<  StatTestOther::FieldToString(FieldToStr("Field2", stat.Field2));

        if (!silentMode)
        {
            std::cout << "CheckString:" << std::endl << "str -- " << str << std::endl
                << "check str -- " << check.str() << std::endl; 
        }
        
        TUT_ASSERT(str == check.str());
    }
    
} // namespace 

// -----------------------------------------------------------
// Tests
namespace 
{
    void SummWithTest(bool silentMode)
    {
        StatTest st1;
        Check(st1, 0, 0, 0, 0);
        
        st1.Field = 5;
        st1.Field2 = 7;
        st1.Field3 = 1.2f;
        st1.Stat1.Add(80);

        StatTest st1Copy = st1;
        st1.SummWith(st1Copy);
        Check(st1, 10, 14, 2.4f, 80);

        // chanding st1 do not affect st1Copy
        Check(st1Copy, 5, 7, 1.2f, 80);
        
        // changing st1Copy do not affect st1
        st1Copy.Stat1.Add(60);
        Check(st1, 10, 14, 2.4f, 80);
        Check(st1Copy, 5, 7, 1.2f, 70);

        StatTest st2;
        st2.SummWith(st1);
        Check(st2, 10, 14, 2.4f, 80);

        if (silentMode) return;

        std::cout << "StatTest as string: " << st1.ToString() << std::endl;
    }

    // -----------------------------------------------------------

    void ClearTest()
    {
        StatTest st1;
        st1.Field = 5;
        st1.Field2 = 7;
        st1.Field3 = 1.2f;
        st1.Stat1.Add(80);

        StatTest st1Copy = st1;
        
        st1.Clear();
        Check(st1, 0, 0, 0, 0);
        Check(st1Copy, 5, 7, 1.2f, 80);
    }
    
    // -----------------------------------------------------------

    void ToStringTest(bool silentMode)
    {
        StatTestOther sto1;
        sto1.Field = 5;
        sto1.Field2 = 10;
        
        std::string sep = "; ";
        CheckString(sto1, sep, silentMode);
        
        if (silentMode) return;

        std::cout << "StatTestOther as string: " << StatTestOther().ToString(sep, false) << std::endl; 
        std::cout << "StatTestOther as string: " << StatTestOther().ToString(sep, true) << std::endl; 
    }
    
} // namespace 

// -----------------------------------------------------------

namespace UtilsTests
{
    void StatisticBaseTest(bool silentMode)
    {
        SummWithTest(silentMode);
        ClearTest();
        ToStringTest(silentMode);
    }
    
} // namespace UtilsTests
