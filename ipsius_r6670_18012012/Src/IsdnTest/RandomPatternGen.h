#ifndef __RANDOMPATTERNGEN__
#define __RANDOMPATTERNGEN__

#include "Utils/Random.h"
#include "Platform/Platform.h"

namespace IsdnTest
{
    using Platform::byte;

    void FillDssPatterns(std::vector<QByteArray> &arr);

    class RandomPatternGen
    {
    public:

        struct Profile
        {
            int Seed;  // random seed

            int ChangeBytesChancePercent;
            int MaxChangeBits;

            int CutBytesChancePercent;
            int MaxCutBytes;

            int AddBytesChancePercent;
            int MaxAddBytes;

            Profile()
            {
                Seed = 0;

                ChangeBytesChancePercent = 75;
                MaxChangeBits = 2 * 8;

                CutBytesChancePercent = 25;
                MaxCutBytes = 3;

                AddBytesChancePercent = 25;
                MaxAddBytes = 3;
            }
        };

        RandomPatternGen(const std::vector<QByteArray> &patterns, 
            const Profile &profile) :
            m_profile(profile),
            m_patterns(patterns)
        {
            ESS_ASSERT(patterns.size() > 0);

            m_rnd.setSeed(m_profile.Seed);
        }

        QVector<byte> Next()
        {
            // select base pattern
            QByteArray v = SelectPattern();

            // change bytes
            v = ChangeBytes(v);

            // cut bytes
            v = CutBytes(v);

            // add bytes
            v = AddBytes(v);

            // return
            QVector<byte> res;
            res.reserve(v.size());
            for(int i = 0; i < v.size(); ++i)
            {
                res.push_back( v.at(i) );
            }

            ESS_ASSERT(res.size() > 0);

            return res;
        }

        static void TestRun(int count)
        {
            Profile profile;

            std::vector<QByteArray> p;
            FillDssPatterns(p);
            //p.push_back("\x10");
            //p.push_back("\x10\x56\xff");

            RandomPatternGen gen(p, profile);

            int t = Platform::GetSystemTickCount();

            int loopCount = count;
            Platform::ddword lengthSum = 0;

            while(loopCount--)
            {
                QVector<byte> v = gen.Next();
                lengthSum += v.size();
            }

            double timeSec = (Platform::GetSystemTickCount() - t) / 1000.0;
            double rate = count / timeSec;
            double avgLength = lengthSum / (double)count;

            std::ostringstream oss;
            oss << "Gen: " << count << " time " << timeSec << " sec. rate " << rate
                << " avg.len " << avgLength;

            std::cout << oss.str() << std::endl;
        }

    private:

        QByteArray SelectPattern()
        {
            int i = m_rnd.Next( m_patterns.size() );
            QByteArray v = m_patterns.at(i);
            ESS_ASSERT(v.size() > 0);
            return v;
        }

        QByteArray AddBytes(QByteArray v)
        {
            if (!Chance(m_profile.AddBytesChancePercent)) return v;

            // 0 .. (size-1); insert beyond pos
            int pos = m_rnd.Next(v.size() + 1) - 1;  
            int bytesCount = 1 + m_rnd.Next(m_profile.MaxAddBytes);

            // generate bytes to add
            QByteArray add;
            for(int i = 0; i < bytesCount; ++i)
            {
                add.push_back(m_rnd.NextByte());
            }

            // make it
            int resultLen = v.size() + bytesCount;
            v = AddBytes(v, pos, add);
            ESS_ASSERT(v.size() == resultLen);

            return v;
        }

        static QByteArray AddBytes(QByteArray v, int pos, const QByteArray &add)
        {
            if (pos < 0) return add + v;
            if (pos >= (v.size() - 1)) return v + add;

            return v.left(pos) + add + v.mid(pos);
        }

        QByteArray CutBytes(QByteArray v)
        {
            if (!Chance(m_profile.CutBytesChancePercent)) return v;

            int startCutPos = m_rnd.Next(v.size());
            int bytesCount = 1 + m_rnd.Next(m_profile.MaxCutBytes);
            
            if (bytesCount >= v.size()) return v;
    
            return v.left(startCutPos) + v.mid(startCutPos + bytesCount);
        }


        QByteArray ChangeBytes(QByteArray v)
        {
            if (!Chance(m_profile.ChangeBytesChancePercent)) return v;

            int startBitPos = m_rnd.Next(v.size() * 8);
            int bitCount = 1 + m_rnd.Next(m_profile.MaxChangeBits);

            // TODO: can be slow, optimise
            while( startBitPos < (v.size() * 8) )
            {
                bool bit = m_rnd.Next(2) == 0 ? false : true; 
                int bytePos = startBitPos / 8;
                int bitNum = startBitPos & 0x07;

                v[bytePos] = SetBit(v.at(bytePos), bitNum, bit);

                // next
                --bitCount;
                if (bitCount == 0) break;

                startBitPos++;
            }
            
            return v;
        }

        static byte SetBit(byte v, byte bitNum, bool bit)
        {
            ESS_ASSERT(bitNum < 8);
            byte mask = (1 << bitNum);

            if (bit) return v | mask;
            return v & (~mask);
        }

        bool Chance(int chancePercent)
        {
            if (chancePercent == 0) return false;

            ESS_ASSERT(chancePercent <= 100);
            int val = m_rnd.Next(100 + 1); // 0 .. 100
            return val <= chancePercent;
        }

        Profile m_profile;
        const std::vector<QByteArray> m_patterns;
        Utils::Random m_rnd;

    };    
    
    
}   // namespace IsdnTest

#endif
