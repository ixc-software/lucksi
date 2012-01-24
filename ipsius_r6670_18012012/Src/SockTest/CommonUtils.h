#ifndef __COMMONUTILS__
#define __COMMONUTILS__

namespace SockTest
{

    class CommUtils
    {

        static char QBitsToHex(byte val)
        {
            ESS_ASSERT(val < 0x10);

            if (val <= 9)
            {
                return '0' + val;
            }

            return 'a' + (val - 10);
        }

        static std::string ByteToHex(byte val)
        {
            std::string res;

            res += QBitsToHex((val >> 4) & 0x0F);
            res += QBitsToHex((val >> 0) & 0x0F);

            return res;
        }

    public:

        static std::string DWToHex(int val, int useBytes = 4)
        {
            ESS_ASSERT((useBytes >= 1) && (useBytes <= 4));

            std::string res;

            res += ByteToHex((val >> 24) & 0xFF);
            res += ByteToHex((val >> 16) & 0xFF);
            res += ByteToHex((val >>  8) & 0xFF);
            res += ByteToHex((val >>  0) & 0xFF);

            // fix length
            res = res.substr((4 - useBytes) * 2);

            return res;
        }


        static std::string DumpVector(const std::vector<byte> &arr, int maxElements = 0)
        {
            using namespace std;
            std::ostringstream dump;

            int count = arr.size();
            bool cutted = false;
            if (maxElements > 0)
            {
                if (count > maxElements)
                {
                    count = maxElements;
                    cutted = true;
                }
            }

            dump << "[" << arr.size() << "] ";

            for(int i = 0; i < count; ++i)
            {
                int val = arr.at(i);
                dump << DWToHex(val, 1) << " ";
            }

            if (cutted) dump << "...";

            return dump.str();
        }
    };

} // namespace SockTest

#endif

