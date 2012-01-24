#include "stdafx.h"
#include "Utils/IntSet.h"
#include "IntSetTests.h"


namespace 
{
    using namespace Utils;
    enum {
        SIZE_SET = 10
    };
    //typedef IntSet<SIZE_SET> TestSet;

    std::string StringToString(const std::string &in)
    {
        IntSet t(SIZE_SET);
        TUT_ASSERT(t.Parse(in));
        return t.ToString(", ", "..");
    }

    void TestParsing(const std::string &in, const std::string &out)
    {
        TUT_ASSERT(StringToString(in) == out);
    }

    void TestGoodParsing()
    {
        IntSet t(SIZE_SET);
        TUT_ASSERT(t.Parse("1..2"));
        TUT_ASSERT(t.Parse("3..4"));
        TUT_ASSERT(t.ToString(", ", "..") == "3, 4");

        TestParsing("", "");      
        TestParsing("2", "2");      
        TestParsing("1, 2", "1, 2");
        TestParsing("1, 2, 3..7, 9", "1..7, 9");
        
        TestParsing("3, 2", "2, 3");        
        TestParsing("1, 2, 3..7, 2, 9", "1..7, 9");
    }
    
    void TestBadParsing()
    {
        IntSet t(SIZE_SET);
        TUT_ASSERT(!t.Parse("-2"));
        TUT_ASSERT(!t.Parse("20"));
        TUT_ASSERT(!t.Parse("5..2"));
        TUT_ASSERT(!t.Parse("1, sadasdfs2"));
        TUT_ASSERT(!t.Parse("1..s3"));
        TUT_ASSERT(t.ToString(", ", "..") == "");
    }

    //------------------------------------------------------

    void TestIsExist()
    {
        IntSet t(SIZE_SET);
        TUT_ASSERT(t.Parse("2") && t.IsExist(2) && !t.IsExist(3));
    }

    //------------------------------------------------------

    void TestClear()
    {
        IntSet t(SIZE_SET);
        TUT_ASSERT(t.Parse("1..3") && t.IsExist(2));
        t.Clear(2);
        TUT_ASSERT(!t.IsExist(2));

        TUT_ASSERT(t.Parse("1..3") && t.IsExist(2));
        t.Clear();
        TUT_ASSERT(!t.Count());
    }

    //------------------------------------------------------

    void TestMaxValue()
    {
        IntSet t(SIZE_SET);
        TUT_ASSERT(t.Parse("1..3") && t.MaxValue() == 3);
        t.Clear();
        TUT_ASSERT(t.Parse("1..3, 4, 10") && t.MaxValue() == 10);
    }

    //------------------------------------------------------

    void TestInclude()
    {
        IntSet t1(SIZE_SET);
        IntSet t2(SIZE_SET);
        TUT_ASSERT(t1.Parse("1..3"));
        TUT_ASSERT(t2.Parse("2..5"));
        t1.Include(t2);
        TUT_ASSERT(t1.ToString(", ", "..") == "1..5");

        TUT_ASSERT(t1.Parse("1..3"));
        TUT_ASSERT(t2.Parse("6..9"));
        t1.Include(t2);
        TUT_ASSERT(t1.ToString(", ", "..") == "1..3, 6..9");
    }

    //------------------------------------------------------

    void TestExclude()
    {
        IntSet t1(SIZE_SET);
        IntSet t2(SIZE_SET);
        TUT_ASSERT(t1.Parse("1..3"));
        TUT_ASSERT(t2.Parse("2..5"));
        t1.Exclude(t2);
        TUT_ASSERT(t1.ToString(", ", "..") == "1");

        TUT_ASSERT(t1.Parse("1..3"));
        TUT_ASSERT(t2.Parse("6..9"));
        t1.Exclude(t2);
        TUT_ASSERT(t1.ToString(", ", "..") == "1..3");
    }

    //------------------------------------------------------

    void TestDifference()
    {
        IntSet t1(SIZE_SET);
        IntSet t2(SIZE_SET);
        IntSet t3(SIZE_SET);

        TUT_ASSERT(t1.Parse("1..3"));
        TUT_ASSERT(t2.Parse("2..5"));
        t3 = t1.Difference(t2);
        TUT_ASSERT(t3.ToString(", ", "..") == "1");

        TUT_ASSERT(t1.Parse("1..3"));
        TUT_ASSERT(t2.Parse("6..9"));
        t3 = t1.Difference(t2);
        TUT_ASSERT(t3.ToString(", ", "..") == "1..3");

        TUT_ASSERT(t1.Parse("1..3"));
        TUT_ASSERT(t2.Parse("1..9"));
        t3 = t1.Difference(t2);
        TUT_ASSERT(t3.ToString(", ", "..") == "");
    }

    //------------------------------------------------------

    void TestIsContain()
    {
        IntSet t1(SIZE_SET);
        IntSet t2(SIZE_SET);
        TUT_ASSERT(t1.Parse("1..3"));
        TUT_ASSERT(t2.Parse("2"));
        TUT_ASSERT(t1.IsContain(t2));
        TUT_ASSERT(t2.Parse("2..5"));
        TUT_ASSERT(!t1.IsContain(t2));
    }

    //------------------------------------------------------

    void TestConvertToVector(const std::string &par)
    {
        IntSet t1(SIZE_SET);
        TUT_ASSERT(t1.Parse(par));
        std::vector<int> v;
        t1.ConvertToVector(v);
        IntSet t2(SIZE_SET, v);        
        TUT_ASSERT(t2 == t1);
    }

    void TestConvertToVector()
    {
        TestConvertToVector("1..3, 5..7");
        TestConvertToVector("1, 2, 3, 5..9");
        TestConvertToVector("2");
    }

	void TestSlice() 
	{
		IntSet t(SIZE_SET);
		TUT_ASSERT(t.Parse("1..3, 5, 6"));
		TUT_ASSERT(t.LowSlice(2).ToString(", ", "..") == "1, 2");
		TUT_ASSERT(t.LowSlice(4).ToString(", ", "..") == "1..3, 5");
		TUT_ASSERT(t.LowSlice(5).ToString(", ", "..") == "1..3, 5, 6");
		TUT_ASSERT(t.HighSlice(3).ToString(", ", "..") == "3, 5, 6");
		TUT_ASSERT(t.HighSlice(5).ToString(", ", "..") == "1..3, 5, 6");
	}

};

namespace UtilsTests
{
    void IntSetTests()
    {
        TestGoodParsing();
        TestBadParsing();
        TestIsExist();
        TestClear();
        TestInclude();
        TestExclude();
        TestDifference();
        TestIsContain();
        TestConvertToVector();
		TestSlice();
    }
}


