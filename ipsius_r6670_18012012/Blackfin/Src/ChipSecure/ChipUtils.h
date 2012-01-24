#ifndef __CHIPUTILS__
#define __CHIPUTILS__

#include "Platform/Platform.h"
#include "Utils/Random.h"

#include "Ds28cn01/ds28cn01.h"

namespace ChipSecure
{
    using Platform::byte;
    using Platform::dword;
    
    struct ChipUtils
    {

        template<int TSize>
        static std::vector<byte> BuffToBytes(const Ds28cn01::Buff<TSize> &buff)
        {
            std::vector<byte> result;
            result.reserve(buff.CSize);

            for(int i = 0; i < buff.CSize; ++i)
            {
                result.push_back( buff[i] );
            }

            ESS_ASSERT(result.size() == buff.CSize);

            return result;
        }

        template<int TSize>
        static void RandomFillBuff(Ds28cn01::Buff<TSize> &buff, Utils::Random &rnd)
        {
            for(int i = 0; i < buff.CSize; ++i)
            {
                buff[i] = rnd.NextByte();
            }
        } 

        template<class T>
        static T ArrayHashT(const std::vector<byte> &v) 
        {        
            T res = 0;
            byte *pRes = (byte*)&res;
            int offs = 0;

            for(int i = 0; i < v.size(); ++i)
            {
                pRes[offs++] ^= v.at(i);
                if (offs >= sizeof(T)) offs = 0;
            }

            return res;
        }

        static dword ArrayHashDw(const std::vector<byte> &v)
        {
            return ArrayHashT<dword>(v);
        }

        static dword ChipTransform(Ds28cn01::DS28CN01 &chip, dword val)
        {
            Ds28cn01::ChallendgeBuffStruct in;
            in.Fill(0xff);

            byte *p = (byte*)&val;
            for(int i = 0; i < sizeof(dword); ++i) in[i] = p[i];

            Ds28cn01::MACBuffStruct result = chip.GetHash(in);
            return ArrayHashDw( BuffToBytes(result) );
        }
        
    };
    
    
}  // namespace ChipSecure

#endif
