/*

  ѕодсчет частоты и мощности сигнала

*/

#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"

#include "get_power.h"
#include "g711codec.h"

using Platform::byte;
using Platform::word;       
using Platform::dword;      

using namespace iDSP;

enum
{
    CMinSamplesCapture      = 256,
};


// -----------------------------------

namespace 
{

    #define MIN_COUNTS  (256)

    // -------------------------------------------------
    
    int GP_Decode(byte val, bool is_A_low)
    {
        return (is_A_low ? alaw2linear(val) : ulaw2linear(val));
    }


    // -------------------------------------------------

    int GP_Sign(int val)
    {
        if (!val) return(0);
        return ((val > 0) ? 1 : -1);
    }

    // -------------------------------------------------

    /*

    ¬ыдел€ет участок (выравненный по нул€м) с размером не меньше minCapture, с четным
    количестовом нулей *pK

    to-do: добавить таймаут дл€ захвата начала и конца,
    при отстутсвии захвата считать только мощность ?

    */

    bool GP_GetPeriod(const byte *pBuff, int count, bool is_A, int minCapture,  // in
                      int *pIndxStart, int *pIndxEnd, int *pK)            // out
    {
        int  curr, prev;
        bool is_zero;
        bool search_start = true;
        
        *pK = 0;

        for (int j = 1; j < count; j++)
        {
            // detect zero
            curr = GP_Decode(pBuff[j], is_A);
            prev = GP_Decode(pBuff[j-1], is_A);
            is_zero = (!curr) || (!(GP_Sign(curr) + GP_Sign(prev)));

            if (!is_zero) continue;

            if (search_start)
            {
                *pIndxStart = j;
                search_start = false;
                continue;
            }

            *pK = *pK + 1;
            if (*pK & 0x01) continue;  // нулей не четное количество

            // get it!
            if (j - *pIndxStart + 1 >= minCapture)
            {
                *pIndxEnd = j;
                return true;
            }
        }

        return false;
    }

    // -------------------------------------------------

    int FloatConv(float val)
    {
        int sign = GP_Sign( (int)val );

        val = (val < 0) ? -val : val;

        int i = (int)(sign * (val + 0.5));

        return i;
    }

    // -------------------------------------------------

    void FillBuff(const byte *pSrc, int srcCount, byte *pDst, int dstCount)
    {
        int srcIndx = 0;

        for (int i = 0; i < dstCount; i++)
        {
            pDst[i] = pSrc[srcIndx++];

            if (srcIndx >= srcCount) srcIndx = 0;
        }

    }

    bool RunSingleTest(const byte *pSrc, int srcCount, bool isALow, int freq, int power, bool print)
    {
        using namespace std;

        if (print)
        {
            cout << (isALow ? "A" : "U") << " " << freq << " " << power << endl;

            for(int i = 0; i < srcCount; ++i) 
            {
                cout << GP_Decode(pSrc[i], isALow) << endl;
            }

            cout << endl;
        }


        byte buff[512];

        FillBuff(pSrc, srcCount, buff, sizeof(buff));

        int resFreq;
        float resPower;
        bool res = GP_ProcessBuff(buff, sizeof(buff), isALow, &resFreq, &resPower);

        if (!res)                           return false;
        if (resFreq != freq)                return false;
        if (FloatConv(resPower) != power)   return false;

        return true;
    }

}  // namespace 


// -----------------------------------


namespace iDSP
{


    bool GP_ProcessBuff(const byte *pBuff, int count, bool is_A,
                        int *pFreq, float *pPower)
    {
        // захват периода
        int  indxStart, indxEnd, K;

        if (!GP_GetPeriod(pBuff, count, is_A, CMinSamplesCapture, &indxStart, &indxEnd, &K))
        {
            return(false);
        }

        // N
        int N = indxEnd - indxStart;
        Platform::ddword sum = 0;

        for (int j = indxStart; j < indxEnd; j++)
        {
            int val = abs( GP_Decode(pBuff[j], is_A) );
            sum += val * val;
        }

        const int CDivider = 16128; // 16208; //  is_A ? CALowDivider : CULowDivider;

        double inSqrt = ( (double)sum ) / N;
        double sumf = sqrt(inSqrt);
        sumf = sumf / CDivider;

        *pPower = 20.0 * log10(sumf); // FloatConv

        *pFreq = (K * 4000UL) / N;

        return true;
    }

    // -------------------------------------------------------------

    bool GP_Test()
    {
        const byte dump_a0[] = {0x3a, 0xd5, 0xba, 0xa3, 0xba, 0xd5, 0x3a, 0x23}; // A, 1K, 0
        const byte dump_a1[] = {0xD5, 0x06, 0x09, 0x06, 0xD5, 0x86, 0x89, 0x86}; // A, 1K, -10
        const byte dump_a2[] = {0x73, 0x4A, 0xD5, 0xCA, 0xF3, 0xCA, 0xD5, 0x4A}; // A, 1K, -30

        const byte dump_m0[] = {0x10, 0xff, 0x90, 0x89, 0x90, 0xff, 0x10, 0x09}; // M, 1K, 0
        const byte dump_m1[] = {0xe6, 0xff, 0x66, 0x5f, 0x66, 0xff, 0xe6, 0xdf}; // M, 1K, -35

        const bool print = false;

        if (!RunSingleTest(dump_m0, sizeof(dump_m0), false, 1000,  0, print)) return false;
        if (!RunSingleTest(dump_m1, sizeof(dump_m1), false, 1000,  -35, print)) return false;

        if (!RunSingleTest(dump_a0, sizeof(dump_a0), true, 1000, 0, print)) return false;
        if (!RunSingleTest(dump_a1, sizeof(dump_a1), true, 1000, -10, print)) return false;
        if (!RunSingleTest(dump_a2, sizeof(dump_a2), true, 1000, -30, print)) return false;

        return true;
    }


}  // namespace iDSP


