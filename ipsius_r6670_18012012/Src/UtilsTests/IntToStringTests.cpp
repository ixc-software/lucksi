
#include "stdafx.h"
#include "IntToStringTests.h"
#include "Utils/IntToString.h"
#include "Utils/ErrorsSubsystem.h"
#include "Platform/PlatformTypes.h"

namespace
{
    using namespace Utils;
    
    /*
    // Returns false if outStrSize less than need
    bool IntToString(int value, char *pOutStr, size_t outStrSize);
    */

    const size_t CBuffSize = 16;
    const size_t CSmallBuffSize = 2; 

    void IntToStringTest()
    {
        // buff size equal
        {
            char buff[CBuffSize];
            int num = -23;
            TUT_ASSERT(IntToString(num, buff, sizeof(buff))); // "-23\0" == 4
            TUT_ASSERT(std::string(buff) == "-23");
            TUT_ASSERT(IntToString(num) == "-23");
        }

        // buff size less than need
        {
            char buff[CSmallBuffSize];
            int num = 1234;
            TUT_ASSERT(!IntToString(num, buff, sizeof(buff))); // "1234\0" == 5
        }

        // buff size bigger than need
        {
            char buff[CBuffSize];
            int num = 2;
            TUT_ASSERT(IntToString(num, buff, sizeof(buff))); // "2\0" == 2
            TUT_ASSERT(std::string(buff) == "2");
            TUT_ASSERT(IntToString(num) == "2");
        }

        // check minAddBytes
        {
            int num = 22;
            TUT_ASSERT(IntToString(num) == "22");
            TUT_ASSERT(IntToString(num, 2) == "22");
            TUT_ASSERT(IntToString(num, 4) == "0022");
        }
    }

    // ------------------------------------------------------------------
    
    /*
    // Returns false if outStrSize less than need
    bool IntToHexString(Platform::dword value, char *pOutStr, size_t outStrSize, 
                        bool add0xPrefix, int minAddBytes);
    std::string IntToHexString(Platform::dword value, bool add0xPrefix = true, int minAddBytes);
    */
    
    void IntToHexStringTest()
    {
        // check size
        {
            char buff[CBuffSize];
            TUT_ASSERT(IntToHexString(0xABCD, buff, sizeof(buff), false, 2));
            TUT_ASSERT(std::string(buff) == "abcd");
        }
        {
            char buff[CSmallBuffSize];
            TUT_ASSERT(!IntToHexString(0xABCD, buff, sizeof(buff), false, 2));
        }
        {
            char buff[CBuffSize + 2];
            TUT_ASSERT(IntToHexString(0xABCD, buff, sizeof(buff), false, 2));
            TUT_ASSERT(std::string(buff) == "abcd");
        }
        
        // check prefix
        {
            char buff[CBuffSize];
            TUT_ASSERT(IntToHexString(0xABCD, buff, sizeof(buff), false, 2));
            TUT_ASSERT(std::string(buff) == "abcd");
            TUT_ASSERT(IntToHexString(0xABCD, false, 2) == "abcd");
        }
        {
            char buff[CBuffSize];
            TUT_ASSERT(IntToHexString(0xABCD, buff, sizeof(buff), true, 2));
            TUT_ASSERT(std::string(buff) == "0xabcd");
            TUT_ASSERT(IntToHexString(0xABCD, true, 2) == "0xabcd");
        }
        {
            char buff[CBuffSize];
            TUT_ASSERT(IntToHexString(0xA, buff, sizeof(buff), true , 1));
            TUT_ASSERT(std::string(buff) == "0x0a");
            TUT_ASSERT(IntToHexString(0xA, true, 1) == "0x0a");
        }

        // check minAddBytes
        {
            TUT_ASSERT(IntToHexString(0x1, false, 0) == "1");
        }
        {
            // minAddBytes > sizeof(dword)
            char buff[CBuffSize];
            TUT_ASSERT(!IntToHexString(0xABCD, buff, sizeof(buff), false, 10));
        }
        {
            char buff[CBuffSize];
            TUT_ASSERT(IntToHexString(0xABCD, buff, sizeof(buff), false, 1));
            TUT_ASSERT(std::string(buff) == "abcd");
            TUT_ASSERT(IntToHexString(0xABCD, false, 1) == "abcd");
        }
        {
            char buff[CBuffSize];
            TUT_ASSERT(IntToHexString(0xABCD, buff, sizeof(buff), false, 2));
            TUT_ASSERT(std::string(buff) == "abcd");
            TUT_ASSERT(IntToHexString(0xABCD, false, 2) == "abcd");
        }
        {
            char buff[CBuffSize];
            TUT_ASSERT(IntToHexString(0xABCD, buff, sizeof(buff), false, 3));
            TUT_ASSERT(std::string(buff) == "00abcd");
            TUT_ASSERT(IntToHexString(0xABCD, false, 3) == "00abcd");
        }
        {
            char buff[CBuffSize];
            TUT_ASSERT(IntToHexString(0xABCD, buff, sizeof(buff), false, 4));
            TUT_ASSERT(std::string(buff) == "0000abcd");
            TUT_ASSERT(IntToHexString(0xABCD, false, 4) == "0000abcd");
        }
        {
            char buff[CBuffSize];
            TUT_ASSERT(IntToHexString(0xA, buff, sizeof(buff), false, 1));
            TUT_ASSERT(std::string(buff) == "0a");
            TUT_ASSERT(IntToHexString(0xA, false, 1) == "0a");
        }
        // check 0
        {
            TUT_ASSERT(IntToHexString(0x0, false, 0) == "0");
            TUT_ASSERT(IntToHexString(0x0, true, 0) == "0x0");
        }
        {
            TUT_ASSERT(IntToHexString(0x0, false, 1) == "00");
            TUT_ASSERT(IntToHexString(0x0, true, 1) == "0x00");
        }
        {
            TUT_ASSERT(IntToHexString(0x10001, false, 1) == "10001");
            TUT_ASSERT(IntToHexString(0x10001, true, 1) == "0x10001");
        }
        // check max dword
        {
            TUT_ASSERT(IntToHexString(0xffffffff, false, 1) == "ffffffff");
            TUT_ASSERT(IntToHexString(0xffffffff, true, 1) == "0xffffffff");
        }
    }

    // ------------------------------------------------------------------

    /*
    void DumpToHexString(void *pData, size_t size, std::string &outStr);
    void DumpToHexString(std::vector<Platform::byte> data, std::string &outStr);
    */
    
    void DumpToHexStringTest()
    {
        {
            Platform::byte array[4] = {0, 1, 2, 3};
            
            std::string str;
            DumpToHexString(array, sizeof(array), str);
            TUT_ASSERT(str == "00 01 02 03");
        }

        {
            std::vector<Platform::byte> vec;
            vec.push_back(5);
            vec.push_back(15);
            vec.push_back(25);

            std::string str;
            DumpToHexString(vec, str);
            TUT_ASSERT(str == "05 0f 19");
        }
    }
    
    // -------------------------------------------------------------------------

    void TestStringToInt()
    {
        int i = 0;

        TUT_ASSERT(Utils::StringToInt("8klsdfj", i) == false);
        TUT_ASSERT(Utils::StringToInt("-", i) == false);
        TUT_ASSERT(Utils::StringToInt("öö-3ó", i) == false);

        TUT_ASSERT(Utils::StringToInt("-122", i) == true && i == -122);
        TUT_ASSERT(Utils::StringToInt("442", i) == true && i == 442);

        TUT_ASSERT(Utils::StringToInt(std::string("8klsdfj"), i) == false);
        TUT_ASSERT(Utils::StringToInt(std::string("-"), i) == false);
        TUT_ASSERT(Utils::StringToInt(std::string("öö-3ó"), i) == false);

        TUT_ASSERT(Utils::StringToInt(std::string("-122"), i) == true && i == -122);
        TUT_ASSERT(Utils::StringToInt(std::string("442"), i) == true && i == 442);

    }

    // -------------------------------------------------------------------------

    void HexStringToIntTest()
    {
        Platform::dword i = 0;

	bool with0xPrefix = false;
        TUT_ASSERT( HexStringToInt("a", i) && i == 0xa);
        TUT_ASSERT( HexStringToInt("a0", i) && i == 0xa0);
        TUT_ASSERT( HexStringToInt("80ffBeda", i) && i == 0x80FFBEDA);
        TUT_ASSERT( HexStringToInt("79", i) && i == 0x79);

        TUT_ASSERT( HexStringToInt("h", i) == false);
        TUT_ASSERT( HexStringToInt("", i) == false);
        TUT_ASSERT( HexStringToInt("17z", i) == false);
        TUT_ASSERT( HexStringToInt("17a", i, with0xPrefix, 4) == false);

        TUT_ASSERT( HexStringToInt("17z", i, with0xPrefix, 2) && i == 0x17);

        // check 0x prefix
	with0xPrefix = true;
        TUT_ASSERT( !HexStringToInt("a", i, with0xPrefix));
        TUT_ASSERT( !HexStringToInt("0a", i, with0xPrefix));
        TUT_ASSERT( !HexStringToInt("xa", i, with0xPrefix));
        TUT_ASSERT( !HexStringToInt("0x", i, with0xPrefix));
        TUT_ASSERT( HexStringToInt("0xA", i, with0xPrefix) && i == 0xA);
        TUT_ASSERT( HexStringToInt("0x0A", i, with0xPrefix) && i == 0x0A);
    }



} // namespace

// ------------------------------------------------------------------

namespace UtilsTests
{
    void IntToStringTests()
    {
        IntToStringTest();
        IntToHexStringTest();
        DumpToHexStringTest();
        TestStringToInt();
        HexStringToIntTest();
    }
    
} // namespace UtilsTests

