
#include "stdafx.h"
#include "IntToString.h"
#include "Utils/ErrorsSubsystem.h"


namespace 
{
    using Platform::dword;
    using Platform::byte;

    const int CDwordBufferSizeBase2 = 32;
    const int CDwordBufferSizeBase8 = 11;
    const int CDwordBufferSizeBase10 = 10;
    const int CDwordBufferSizeBase16 = 8;

    // -------------------------------------------------------------------------
    
    void strreverse(char* begin, char* end) 
    {
        char aux;

        while (end>begin)
            aux=*end, *end--=*begin, *begin++=aux;

    }

    // -------------------------------------------------------------------------

    // Ansi C "itoa" based on Kernighan & Ritchie's "Ansi C":
    void c_itoa(int value, char* str, int base)
    {
        static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";

        char* wstr=str;
        int sign;

        // Validate base
        if (base<2 || base>35)
        {
            *wstr='\0'; return;
        }

        // Take care of sign
        if ((sign=value) < 0) value = -value;

        // Conversion. Number is reversed.
        do *wstr++ = num[value%base]; while (value/=base);

        if (sign<0) *wstr++='-';

        *wstr='\0';

        // Reverse string
        strreverse(str,wstr-1);
    }

    // -------------------------------------------------------------------------

    // Returns significant digits count
    int IntToHexReversedStr(dword value, char *outStr)
    {
        char *pRes = outStr; 

        int count = 1;
        for (int i = 0; i < CDwordBufferSizeBase16; ++i)
        {
            byte digit = value & 0xf;
            if (digit <= 9) *pRes = '0' + digit; 
            if ((digit >= 10) && (digit <= 15)) *pRes = 'a' + digit - 10; 

            ++pRes;
            
            value = value >> 4;
            if (value > 0)  ++count;
        }

        *pRes = '\0';

        return count;
    }
        
} // namespace

// -------------------------------------------------------------------------

namespace Utils
{
    bool IntToString(int value, char *pOutStr, size_t outStrSize)
    {
        if (outStrSize < (CDwordBufferSizeBase10 + 1)) return false; // +1 for '\0'

        c_itoa(value, pOutStr, 10);

        return true;
    }

    // -------------------------------------------------------------------------

    std::string IntToStringSign(int val)
    {
        std::string res;
        if (val > 0) res += "+";
        res += IntToString(val);
        return res;
    }

    // -------------------------------------------------------------------------

    std::string IntToString(int value, int minAddBytes)
    {
        char buff[CDwordBufferSizeBase10];

        ESS_ASSERT(IntToString(value, buff, sizeof(buff) + 1));

        std::string res(buff);
        if (res.size() >= minAddBytes) return res;
        
        return std::string((minAddBytes - res.size()), '0') + res;
    }

    // -------------------------------------------------------------------------
    
    bool IntToHexString(Platform::dword value, 
                        char *pOutStr, 
                        size_t outStrSize, 
                        bool add0xPrefix, 
                        int minAddBytes)
    {
        size_t prefixSize = (add0xPrefix) ? 2 : 0; // "0x"
        int minAddDigits = minAddBytes * 2;

        if (outStrSize < (CDwordBufferSizeBase16 + prefixSize + 1)) return false; // +1 for '\0'
        if (minAddBytes > sizeof(Platform::dword)) return false;
        
        char *pStrBegin = pOutStr;

        if (add0xPrefix)
        {
            *pOutStr++ = '0';
            *pOutStr++ = 'x';
        }

        char buff[CDwordBufferSizeBase16 + 1];
        int addDigits = IntToHexReversedStr(value, buff);
        if (addDigits < minAddDigits) addDigits = minAddDigits;
        
        for (int i = addDigits - 1; i >= 0; --i)
        {
            *pOutStr++ = buff[i];
        }

        *pOutStr = '\0';
        
        pOutStr = pStrBegin;
        
        return true;
    }

    // -------------------------------------------------------------------------
    
    std::string IntToHexString(Platform::dword value, bool add0xPrefix, int minAddBytes)
    {
        char buff[CDwordBufferSizeBase16 + 3]; // + prefix and \0

        ESS_ASSERT(IntToHexString(value, buff, sizeof(buff), add0xPrefix, minAddBytes));

        return std::string(buff);
    }

    // -------------------------------------------------------------------------
    
    void DumpToHexString(const void *pData, size_t size, std::string &outStr)
    {
        ESS_ASSERT(pData != 0);        
        outStr.clear();
        if (size == 0) return;

        std::string sep(" ");
        int useBytes = 1;
        const Platform::byte *pBytes = static_cast<const Platform::byte*>(pData);
        
        for (int i = 0; i < size; ++i)
        {
            outStr += IntToHexString(*pBytes++, false, useBytes);
            if (i != (size - 1)) outStr += sep;
        }
    }

    // -------------------------------------------------------------------------
    
    void DumpToHexString(const std::vector<Platform::byte> &data, std::string &outStr)
    {
        outStr.clear();

        if (data.size() == 0) return;
        
        DumpToHexString(&data.at(0), data.size(), outStr);
    }

    // -------------------------------------------------------------------------

    std::string DumpToHexString(const void *pData, size_t size)
    {
        std::string res;
        DumpToHexString(pData, size, res);

        return res;
    }

    // -------------------------------------------------------------------------

    std::string DumpToHexString(const std::vector<Platform::byte> &data)
    {
        return DumpToHexString(&data.at(0), data.size());
    }

    // -------------------------------------------------------------------------

    // Returns true if convert ok
    bool StringToInt(const char *string, int &value)
    {
        value = 0;
        int sign = 1;

        if (!string || !*string ) 
            return false;

        if ( *string == '-') 
        {
            sign = -1;
            string++;
        }
        if (!*string ) 
            return false;

        while (*string) 
        {
            if ( (*string >= '0') && (*string <= '9') ) 
            {
                value = value * 10 + (*string - '0');
                string++;
                continue;
            }
            return false;
        }
        value *= sign; 
        return true;
    }
    
    // -------------------------------------------------------------------------

    // Returns true if convert ok
    bool StringToInt(const std::string &str, int &value)
    {
        return StringToInt(str.c_str(), value);
    }

    // -------------------------------------------------------------------------
    
    bool StringToInt(const std::string &str, Platform::dword &value, bool allowNegative)
    {
        int num = 0;
        bool ok = StringToInt(str, num);
        if (!ok) return false;

        if (!allowNegative && (num < 0)) return false;

        value = (dword)num;

        return true;
    }

    // -------------------------------------------------------------------------

    int HexCharToInt(char c)
    {
        if ((c >= '0') && (c <= '9')) return c - '0';
        if ((c >= 'a') && (c <= 'f')) return c - 'a' + 10;
        if ((c >= 'A') && (c <= 'F')) return c - 'A' + 10;
        return -1;
    }

    bool HexStringToInt(const char *pData, 
                        Platform::dword &value, 
                        bool with0xPefix, 
                        int dataCount)
    {
        if (pData == 0) return false;
        if (*pData == 0) return false;

        // check and skip prefix
        if (with0xPefix)
        {
            if (*pData++ != '0') return false;
            if (*pData++ != 'x') return false;
            if (*pData == 0) return false;
        }

        value = 0;
        int chCount = 0;

        while(true)
        {
            char c = *pData++;
            chCount++;
            if ((dataCount > 0) && (chCount > dataCount)) break;
            if (c == 0) 
            {
                if (dataCount > 0) return false;
                break;
            }
            if (chCount > 8) return false;

            int fourBits = HexCharToInt(c);
            if (fourBits < 0) return false;

            value = value << 4;
            value |= (fourBits & 0x0f);
        }

        return true;
    }

    bool HexStringToInt(const std::string &s, Platform::dword &value, bool with0xPefix)
    {
        return HexStringToInt(s.c_str(), value, with0xPefix);
    }

    
} // namespace Utils
