#ifndef __STRINGUTILSW__
#define __STRINGUTILSW__

namespace Utils
{
	
    std::wstring PadLeft(const std::wstring &s, int length, char c);  
    std::wstring PadRight(const std::wstring &s, int length, char c);  

    // Dirty, must using encoding
    std::wstring StringToWString(const std::string &s);
    std::string  WStringToString(const std::wstring &s);
	
}  // namespace Utils

#endif

