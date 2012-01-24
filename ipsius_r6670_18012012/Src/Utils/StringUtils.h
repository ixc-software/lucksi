#ifndef __STRINGUTILS__
#define __STRINGUTILS__

namespace Utils
{
	
    std::string PadLeft(const std::string &s, int length, char c);  
    std::string PadRight(const std::string &s, int length, char c);  

    std::string TrimBlanks(const std::string &s);

    bool StringStartWith(const std::string &s, const std::string &val);

    // in and out can be same object, must be: in.size() == out.size()
    // return true, if all characters < 0x80
    bool StringUpCase(const std::string &in, std::string &out);
    std::string StringUpCase(const std::string &s, bool *pAllCharsOK = 0);

    // notes is same as notes for StringUpCase()
    bool StringLowerCase(const std::string &in, std::string &out);
    std::string StringLowerCase(const std::string &s, bool *pAllCharsOK = 0);

    void StringCaseConvTest();


}  // namespace Utils

#endif

