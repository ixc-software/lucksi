
#include "stdafx.h"
#include "StackStringTest.h"
#include "Utils/StackString.h"

namespace
{
    using namespace Utils;
    
    /*
    StackStringT();
    StackStringT(const char *p);
    StackStringT(const std::string &s);
    */
    
    void TestSimpleConstructors()
    {
        {
            StackString empty;
            TUT_ASSERT(empty.Length() == 0);
            TUT_ASSERT(!empty.Overflowed());
        }
        
        {
            const size_t buffLength = 6;
            char buff[buffLength] = {'a', 'b', 'c', 'd', 'e', '\0'};
            
            StackString str1(buff);
            TUT_ASSERT(str1.Length() == (buffLength - 1));
            TUT_ASSERT(!str1.Overflowed());
            TUT_ASSERT(std::string(buff) == std::string(str1.c_str()));
        }

        {
            std::string buff("string");
            StackString str2(buff);
            TUT_ASSERT(str2.Length() == buff.size());
            TUT_ASSERT(std::string(str2.c_str()) == buff);
        }
    }

    // ----------------------------------------------------
    
    /*
    template<int OtherBuffSize> StackStringT(const StackStringT<OtherBuffSize> &other);
    template<int OtherBuffSize> void operator=(const StackStringT<OtherBuffSize> &other);
    */

    void TestCopyConstructorAndCopyAssigment()
    {
        std::string buff1("abcd");
        std::string buff2("def");

        // test copy constructor
        {
            StackString str(buff1);
            StackString copyStr(str);
            TUT_ASSERT(str == copyStr);
        }

        // test copy assigment
        {
            StackString str0;
            StackString str1(buff1);
            StackString str2(buff2);

            str0 = str1;
            TUT_ASSERT(str0 == str1);
            TUT_ASSERT(std::string(str0.c_str()) == buff1);

            str1 = str2;
            TUT_ASSERT(str1 == str2);
            TUT_ASSERT(std::string(str1.c_str()) == buff2);

            str2 = StackString();
            TUT_ASSERT(str2.Length() == 0);
        }
    }
    
    // ----------------------------------------------------

    /*
    int Compare(const char *p);
    template<int OtherBuffSize> int Compare(const StackStringT<OtherBuffSize> &other);
    operators ==, !=, >, >=, <, <=
    */
    
    void TestCompare()
    {
        std::string same("string");
        std::string less("str");
        std::string bigger("string1");
        
        StackString str(same);
        
        TUT_ASSERT(str.Compare(same.c_str()) == 0);
        TUT_ASSERT(str.Compare(less.c_str()) > 0);
        TUT_ASSERT(str.Compare(bigger.c_str()) < 0);

        TUT_ASSERT(str.Compare(StackString(same)) == 0);
        TUT_ASSERT(str.Compare(StackString(less)) > 0);
        TUT_ASSERT(str.Compare(StackString(bigger)) < 0);
        
        
        TUT_ASSERT(str == StackString(same));
        TUT_ASSERT(str <= StackString(same));
        TUT_ASSERT(str >= StackString(same));
        
        TUT_ASSERT(str > StackString(less));
        TUT_ASSERT(str >= StackString(less));

        TUT_ASSERT(str < StackString(bigger));
        TUT_ASSERT(str <= StackString(bigger));
        TUT_ASSERT(str != StackString(bigger));
    }
    
    // ----------------------------------------------------

    /*
    template<int OtherBuffSize> void Add(const StackStringT<OtherBuffSize> &other);
    void Add(const std::string &s);
    void Add(const char *p);
    void AddInt(int i);
    void AddHex(int i, int bytesToAdd = 4);

    template<int OtherBuffSize> void operator+=(const StackStringT<OtherBuffSize> &other);
    void operator+=(const char *p);
    void operator+=(const std::string &s);
    */
    
    void TestAddition()
    {
        {
            std::string data1("string");
            std::string data2("string");
            StackString str;
            str.Add(data1);
            str.Add(data1.c_str());
            str.Add(StackString(data2));
            TUT_ASSERT(std::string(str.c_str()) == (data1 + data1 + data2));
        }
        {
            int num1 = 5;
            StackString str1;
            str1.AddInt(num1);
            TUT_ASSERT(std::string(str1.c_str()) == "5");
            str1.AddInt(num1);
            TUT_ASSERT(std::string(str1.c_str()) == "55");

            str1.Clear();
            str1.AddInt(-10);
            TUT_ASSERT(std::string(str1.c_str()) == "-10");
        }

        {
            int num2 = 256; // 0x100
            StackString str2;
            str2.AddHex(num2);
            TUT_ASSERT(std::string(str2.c_str()) == "00000100"); // use atleast 4 bytes

            str2.Clear();
            str2.AddHex(num2, 1);
            TUT_ASSERT(std::string(str2.c_str()) == "100"); // use atleast 1 byte

            str2.Clear();
            str2.AddHex(0xabcde);
            TUT_ASSERT(std::string(str2.c_str()) == "000abcde"); // use atleast 4 bytes
        }
        
        {
            StackString str3("ab");
            str3 += "c";
            str3 += std::string("de");
            str3 += StackString("f");

            TUT_ASSERT(std::string(str3.c_str()) == "abcdef");
        }
    }
    
    // ----------------------------------------------------

    /*
    bool Overflowed() const;;
    */
    
    void TestOwerflow()
    {
        typedef StackStringT<4> TestOverflowString;

        {
            TestOverflowString empty;
            TUT_ASSERT(!empty.Overflowed());
        }

        // instant overflowing
        {
            TestOverflowString str1("string");
            TUT_ASSERT(str1.Overflowed());

            str1.Clear();
            TUT_ASSERT(!str1.Overflowed());

            str1.Add("buffer");
            TUT_ASSERT(str1.Overflowed());
        }

        // gradient overflowing
        {
            TestOverflowString str2("a");
            TUT_ASSERT(!str2.Overflowed());

            str2.Add("b");
            TUT_ASSERT(!str2.Overflowed());

            str2 += "cd";
            TUT_ASSERT(!str2.Overflowed());

            str2 += "e";
            TUT_ASSERT(str2.Overflowed());
        }

        // copy overflowed buffer to smaller and bigger
        {
            TestOverflowString smallStr("ab");
            TestOverflowString overflowedStr("string");

            smallStr = overflowedStr;
            TUT_ASSERT(smallStr.Overflowed());

            StackString bigStr;
            bigStr = overflowedStr;
            TUT_ASSERT(!bigStr.Overflowed());
        }
    }
    
    // ----------------------------------------------------

    /*
    void Clear();
    int Length() const;
    const char* c_str();
    char& operator[](int indx);
    */
    
    void TestOther()
    {
        std::string data("string");
        
        StackString str;
        TUT_ASSERT(str.Length() == 0);
        TUT_ASSERT(str.c_str() != 0);
        
        str.Clear();
        TUT_ASSERT(str.Length() == 0);
        TUT_ASSERT(str.c_str() != 0);

        str.Add(data);
        TUT_ASSERT(str.Length() == data.size());
        TUT_ASSERT(str.c_str() != 0);
        TUT_ASSERT(std::string(str.c_str()) == data);

        str.Clear();
        TUT_ASSERT(str.Length() == 0);

        str.Add(data);
        for (size_t i = 0; i < data.size(); ++i)
        {
            TUT_ASSERT(str[i] == data.at(i));
        }

        str[0] = 'd';
        TUT_ASSERT(str[0] == 'd');
    }

    // ----------------------------------------------------

    /*
    template<int OtherBuffSize> StackStringT(const StackStringT<OtherBuffSize> &other); 
    template<int OtherBuffSize> void operator=(const StackStringT<OtherBuffSize> &other);
    template<int OtherBuffSize> void Add(const StackStringT<OtherBuffSize> &other);
    template<int OtherBuffSize> int Compare(const StackStringT<OtherBuffSize> &other);
    template<int OtherBuffSize> void operator+=(const StackStringT<OtherBuffSize> &other);
    */

    void TestWithDiffBufferSizes()
    {
        typedef StackStringT<50> BigStackString;
        typedef StackStringT<4> SmallStackString;

        {
            BigStackString big("string");
            SmallStackString small(big);
            TUT_ASSERT(small.Overflowed());
        }
        
        {
            SmallStackString small("string"); // oveflowed
            BigStackString big = small;
            TUT_ASSERT(!big.Overflowed());
        }
 
        {
            SmallStackString small("string"); // oveflowed
            BigStackString big("str");
            
            BigStackString test1;
            test1.Add(small);
            TUT_ASSERT(!test1.Overflowed());
            test1 += small;
            TUT_ASSERT(!test1.Overflowed());

            SmallStackString test2;
            test2.Add(big);
            TUT_ASSERT(!test2.Overflowed());
            test2.Add(big);
            TUT_ASSERT(test2.Overflowed());
        }
        
        {
            SmallStackString str1("str");
            BigStackString str2("str");
            BigStackString str3("st");
            
            TUT_ASSERT(str1.Compare(str2) == 0); // despite buffers sizes
            TUT_ASSERT(str1.Compare(str3) > 0);
        }
        
    }

    
} // namespace 

// ----------------------------------------------------

namespace UtilsTests
{
    using namespace Utils;
    
    void StackStringTest()
    {
        TestSimpleConstructors();
        TestCopyConstructorAndCopyAssigment();
        TestCompare(); 
        TestAddition();
        TestOwerflow();
        TestOther();
        TestWithDiffBufferSizes();
    }
    
} // namespace UtilsTests 
