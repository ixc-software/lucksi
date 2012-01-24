#ifndef __STRINGUTILSIMPL__
#define __STRINGUTILSIMPL__

namespace Utils
{
	
    template<class TString>
    void PadString(const TString &s, int length, bool padLeft, char c, TString &result)  // get char type from TString - ?!
    {
        result.clear();
        result += s;

        int count = length - result.length();
        int pos = padLeft ? 0 : s.size();        
        if (count > 0) result.insert(pos, count, c);
    }
   			
}  // namespace Utils


#endif

