#include "stdafx.h"

#include "Utils/Random.h"

#include "FlashTest.h"

// ------------------------------------

using Platform::byte;
using Platform::dword;

namespace
{
    using namespace ExtFlash;

    bool EraseFlash(IFlashTestIntf &flash, IFlashTestCallback &callback)
    {
        int sectors = flash.Map().Sectors;

        for(int i = 0; i < sectors; ++i)        
        {
            if (!flash.SectorErase(i)) return false;
            callback.SectorErased(i);
        }
        
        return true;
    }

    void RunCycle(IFlashTestIntf &flash, IFlashTestCallback &callback, bool direct)
    {
        if (!EraseFlash(flash, callback)) return;
        RunSingleFlashStage(flash, callback, direct);
    }

    void FillOriginalData(std::vector<byte> &buff, int size)
    {
        buff.resize(size);

        Utils::Random rnd(11);
        for(int i = 0; i < size; ++i) buff.at(i) = rnd.NextByte();
    }

    void TransformData(const std::vector<byte> &origBuff, int page, bool direct, 
        std::vector<byte> &result)
    {
        ESS_ASSERT(origBuff.size() > 0);
        ESS_ASSERT(origBuff.size() % sizeof(dword) == 0);

        result.resize(origBuff.size());

        dword *pIn  = (dword*)(&origBuff[0]);
        dword *pOut = (dword*)(&result[0]);
        int count = origBuff.size() / sizeof(dword);

        while(count--)
        {
            dword val = *pIn++;
            val = val ^ page;
            if (!direct) val = ~val;
            *pOut++ = val;
        }
    }

}   // namespace


// ------------------------------------

namespace ExtFlash
{
    
    void RunFullFlashTest(IFlashTestIntf &flash, IFlashTestCallback &callback)
    {
        RunCycle(flash, callback, true);        
        RunCycle(flash, callback, false);        
    }

    void RunSingleFlashStage(IFlashTestIntf &flash, IFlashTestCallback &callback, bool directCycle)
    {
        int pageSize = flash.Map().PageSize;
        int pages = flash.Map().Pages;

        std::vector<byte> origBuff;
        FillOriginalData(origBuff, pageSize);

        std::vector<byte> pageBuff;
        pageBuff.resize(pageSize);

        // write
        {
            for(int i = 0; i < pages; ++i)
            {
                TransformData(origBuff, i, directCycle, pageBuff);
                bool ok = flash.PageWrite(i, &pageBuff[0], pageSize);
                callback.PageWrited(i, !ok);
            }
        }

        // verify
        {
            std::vector<byte> readBuff;
            readBuff.resize(pageSize);
            
            for(int i = 0; i < pages; ++i)
            {
                bool ok = flash.PageRead(i, &readBuff.at(0), pageSize);

                if (ok)
                {
                    TransformData(origBuff, i, directCycle, pageBuff);

                    // verify
                    for(int i = 0; i < pageSize; ++i)
                    {
                        if (readBuff.at(i) != pageBuff.at(i))
                        {
                            ok = false;
                            break;
                        }
                    }
                }

                callback.PageVerified(i, !ok);
            }        
        }
    }
    
}  // namespace ExtFlash

