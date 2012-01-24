#ifndef RANGEIOBASE_H
#define RANGEIOBASE_H

#include "SectorStorageDeviceBase.h"
#include "FlashIOStream.h"
#include "StoreRange.h"


namespace BfBootSrv
{
    using boost::shared_ptr;
    using boost::scoped_ptr;
    using Platform::dword;

    // For use in test only.
    class IDebugRangeIOBase : Utils::IBasicInterface
    {        
    public:        
        virtual const StoreRange& getRange(int) = 0;
    };

    // Класс позваляющий выполнять IO-операции на флеши в рамках регионов.
    class RangeIOBase : public IDebugRangeIOBase
    {
    public:
        enum NamedRangeIndex
        {
            min = -1,

            CfgPrim = 0,
            CfgSecond,
            CfgDefault,
            AppStorRange,             

            max
        };

        shared_ptr<FlashIStream> InStreamRange(NamedRangeIndex index);
        shared_ptr<FlashOStream> OutStreamRange(NamedRangeIndex index);

        shared_ptr<FlashIStream> InStreamAll();
        shared_ptr<FlashOStream> OutStreamAll();

        int getFullSize() const;
        void EraseRange(NamedRangeIndex regionIndex); 


    //protected:
    public:
        
        RangeIOBase(SectorStorageDeviceBase& flash);
        // Добавляемый регион будет расположен на карте сеторов непосредственно после предыдущего.
        // Порядок должен соответствовать порядку в енуме
        void BindNextRegion(NamedRangeIndex region, int sectorCount);
        void BindRegion(NamedRangeIndex region, int startSector, int sectorCount);

    private: 

        shared_ptr<FlashOStream> getOut(const StoreRange& range);
        shared_ptr<FlashIStream> getIn(const StoreRange& range) const;
        void AssertRegion(NamedRangeIndex region) const;

    private:
        const StoreRange& getRange(int index);
        
        std::vector<StoreRange> m_map;   
        NamedRangeIndex m_lastAdded;
        SectorStorageDeviceBase& m_flash;
        StoreRange m_totalRange;
    };    

   
} // namespace BfBootSrv

#endif
