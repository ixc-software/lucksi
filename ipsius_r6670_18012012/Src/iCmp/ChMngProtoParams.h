#ifndef __CHMNGPROTOPARAMS__
#define __CHMNGPROTOPARAMS__

#include "Utils/IntToString.h"

namespace iCmp
{
    
    struct ChMngProtoParams
    {

        static const char* CTrue()   {  return "true";  }
        static const char* CFalse()  {  return "false"; }

        static const char* CSep()  {  return ";"; }

        static std::string ToString(bool val)
        {
            return val ? CTrue() : CFalse();
        }

        static std::string ToString(int val)
        {
            return Utils::IntToString(val);
        }

        static std::string ToString(const std::string &s) { return s; }

        static bool FromString(const std::string &s, bool &val)
        {
            if (s == CTrue())
            {
                val = true;
                return true;
            }

            if (s == CFalse())
            {
                val = false;
                return true;
            }

            return false;
        }

        static bool FromString(const std::string &s, int &val)
        {
            return Utils::StringToInt(s, val);
        }

        static bool FromString(const std::string &s, std::string &val)
        {
            val = s;
            return true;
        }
        
    public:

        template<class T>
        static std::string ToParams(T val)
        {
            return ToString(val);
        }

        template<class T1, class T2>
        static std::string ToParams(T1 val1, T2 val2)
        {
            return ToString(val1) + CSep() + ToString(val2);
        }

        template<class T>
        static bool FromParams(const std::string &p, T &val)
        {
            return FromString(p, val);
        }

        template<class T1, class T2>
        static bool FromParams(const std::string &p, T1 &val1, T2 &val2)
        {
            int pos = p.find( CSep() );
            if (pos < 0) return false;
            return FromString(p.substr(0, pos),  val1) && 
                   FromString(p.substr(pos + 1), val2);
        }

    };
    
}  // namespace iCmp

#endif
