
#ifndef __UTILS_INTTOSTRING__
#define __UTILS_INTTOSTRING__

// IntToHexString.h

#include "Platform/PlatformTypes.h"


namespace Utils
{
    // Returns false if outStrSize less than need  
    bool IntToString(int value, char *pOutStr, size_t outStrSize);

    std::string IntToString(int value, int minAddBytes = 1);
    std::string IntToStringSign(int val);

    // Returns false if outStrSize less than need
    bool IntToHexString(Platform::dword value, 
                        char *pOutStr, 
                        size_t outStrSize, 
                        bool add0xPrefix = true, 
                        int minAddBytes = 4);
    
    std::string IntToHexString(Platform::dword value, 
                               bool add0xPrefix = true, 
                               int minAddBytes = 4);
    
    void DumpToHexString(const void *pData, size_t size, std::string &outStr);

    void DumpToHexString(const std::vector<Platform::byte> &data, std::string &outStr);

    std::string DumpToHexString(const void *pData, size_t size);

    std::string DumpToHexString(const std::vector<Platform::byte> &data);

    // Returns true if convert ok
    // atoi can't handle errors
    bool StringToInt(const char *str, int &value);

    // Returns true if convert ok
    bool StringToInt(const std::string &str, int &value);
    
    bool StringToInt(const std::string &str, Platform::dword &value, bool allowNegative);
    
    // Returns true if convert ok
    // if (dataCount == 0) -> pData length auto detected
    bool HexStringToInt(const char *pData, 
                        Platform::dword &value, 
                        bool with0xPefix = false, 
                        int dataCount = 0);
    
    bool HexStringToInt(const std::string &s, 
                        Platform::dword &value, 
                        bool with0xPefix = false);
   
} // namespace Utils

#endif

