#include "stdafx.h"

#include "Utils/Random.h"

#include "CyclicBuff.h"

// debug
#include "FreqFixer.h"
/*
#include "MixerPoints.h"
#include "MixerCore.h"  
*/

namespace SndMix
{
    
    /* 


        ---


        * Active feed callback on reading (for gen) - ?

        Freq adoptation -- только после полного прохождения тестов и проверки миксера 

        #1. Каждый N1 rd сэмплов посмотреть на разницу rd_total - wr_total 
        тренд -- если увеличивается, то K += 1, иначе K -=1 (порог игнорирования, схожесть)
        #2. K либо добавляет +1 отчет на N2 write отсчетов, либо убирает. 
        * или регулировать период Z, через который происходит добавление или удаление одного отсчета   

    */

    // return vector [from, to]
    static std::vector<int16> VecFill(int from, int to)
    {
        ESS_ASSERT(from <= to);

        std::vector<int16> res;

        while(from <= to)
        {
            res.push_back(from++);
        }

        return res;
    }

    void CycBuffTestNormal()
    {
        const int CBuffSize = 32;

        CyclicBuff cb(CBuffSize);

        Utils::Random rnd(1);
        int loopCount = 1024;
        int totalData = 0;

        while(loopCount--)
        {
            int amount = rnd.Next(CBuffSize / 2) + 1;
            totalData += amount;

            // write
            {
                std::vector<int16> block;
                for(int i = 0; i < amount; ++i) block.push_back(i);
                cb.WriteBlock(block);
            }

            // read
            {
                CyclicBuff::RdIter i(cb, amount);

                for(int j = 0; j < amount; ++j)
                {
                    int val = i.ReadAndMove();
                    ESS_ASSERT(val == j);
                }

                ESS_ASSERT( i.End() );
            }
        }

        CyclicBuffState state = cb.GetState();
        ESS_ASSERT(state.DataRead   == totalData);
        ESS_ASSERT(state.DataWrite  == totalData);
        ESS_ASSERT( state.NoErrors() );
    }

    // ------------------------------------------------------------

    void CycBuffWriteOverrunTest()
    {
        using namespace std;
        const bool CDebug = false;

        CyclicBuff cb(8);

        // rd/wr +4
        {
            std::vector<int16> data(4, 0);
            cb.WriteBlock(data);
            cb.ReadBlock(data, data.size());

            if (CDebug) cout << cb.DebugInfo() << endl;
        }

        // wr +7 (0 .. 6), no overrun
        {
            cb.WriteBlock( VecFill(0, 6) );
            ESS_ASSERT( cb.GetState().NoErrors() );

            if (CDebug) cout << cb.DebugInfo() << endl;
        }

        // wr +4 (6 .. 9), wr overrun
        {
            cb.WriteBlock( VecFill(6, 9) );
            if (CDebug) cout << cb.DebugInfo() << endl;

            ESS_ASSERT( cb.GetState().WriteErrors       == 1 );
            ESS_ASSERT( cb.GetState().WriteErrDataCount == 4 );
        }

        // read +7
        {
            const int CSize = 7;
            int16 ref[CSize] = { 0, 1, 2, 6, 7, 8, 9 };
            
            std::vector<int16> data;
            cb.ReadBlock(data, CSize);            

            std::vector<int16> refData(&ref[0], &ref[0] + CSize);
            ESS_ASSERT(data == refData);
        }

    }

    // ------------------------------------------------------------

    void CycBuffReadOverrunTest()
    {
        CyclicBuff cb(8);

        // rd/wr +4
        {
            std::vector<int16> data(4, 9);
            cb.WriteBlock(data);
            cb.ReadBlock(data, data.size());
        }

        // wr +5 (0 .. 4), no overrun
        {
            cb.WriteBlock( VecFill(0, 4) );
            ESS_ASSERT( cb.GetState().NoErrors() );
        }

        // read +7, overrun
        {
            const int CSize = 7;
            int16 ref[CSize] = { 9, 9, 0, 1, 2, 3, 4 };

            std::vector<int16> data;
            cb.ReadBlock(data, CSize);            

            std::vector<int16> refData(&ref[0], &ref[0] + CSize);
            ESS_ASSERT(data == refData);

            ESS_ASSERT( cb.GetState().ReadErrors       == 1 );
            ESS_ASSERT( cb.GetState().ReadErrDataCount == 2 );
        }

    }
        
    
}  // namespace SndMix 