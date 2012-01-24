#ifndef __PCUTILS__
#define __PCUTILS__

#include "Platform/Platform.h"
#include "CommonUtils.h"

namespace SockTest
{
    using Platform::byte;
    using Platform::dword;

    // Some help methods
    class PcUtils
    {

    public:

        static std::vector<byte> FromQByteArr(const QByteArray &arr)
        {
            std::vector<byte> res;
            res.reserve(arr.length());

            for(int i = 0; i < arr.length(); ++i)
            {
                res.push_back(arr.at(i));
            }

            return res;
        }

        static QByteArray ToQByteArr(const std::vector<byte> &arr)
        {
            QByteArray res;
            res.reserve(arr.size());

            for(int i = 0; i < arr.size(); ++i)
            {
                res.push_back(arr.at(i));
            }

            return res;
        }

        static std::string DumpQByteArray(const QByteArray &arr, int maxElements = 0)
        {
            return CommUtils::DumpVector( FromQByteArr(arr), maxElements );
        }

    };

}  // namespace SockTest

#endif
