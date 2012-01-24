#ifndef __FLASHTEST__
#define __FLASHTEST__

#include "Utils/IBasicInterface.h"

#include "flashMap.h"

namespace ExtFlash
{

    class IFlashTestIntf : public Utils::IBasicInterface
    {
    public:

        virtual const FlashMap& Map() const = 0;
        virtual bool SectorErase(int sectorNumber) = 0;
        virtual bool PageRead(int page, void *p, int size) = 0;
        virtual bool PageWrite(int page, const void *p, int size) = 0;
    };

    // --------------------------------------------

    class IFlashTestCallback : public Utils::IBasicInterface
    {
    public:
        virtual void SectorErased(int sectorNumber) = 0;
        virtual void PageWrited(int pageNumber, bool withError) = 0;
        virtual void PageVerified(int pageNumber, bool withError) = 0;
    };

    // --------------------------------------------

    void RunFullFlashTest(IFlashTestIntf &flash, IFlashTestCallback &callback);

    // must called with flash erased
    // for complete test, call twice, with directCycle = true and = false
    void RunSingleFlashStage(IFlashTestIntf &flash, IFlashTestCallback &callback, bool directCycle);
    
}  // namespace ExtFlash

#endif

