#ifndef FLASHIOSTREAM_H
#define FLASHIOSTREAM_H

#include "Utils/BinaryReader.h"
#include "Utils/BinaryWriter.h"
#include "Utils/IBiRWStream.h" 
#include "Utils/SafeRef.h"


#include "SectorStorageDeviceBase.h"

namespace BfBootSrv
{
    ESS_TYPEDEF_FULL(OutOfRange, Utils::NotEnoughMemory); 
    ESS_TYPEDEF_FULL(NoFreeSectors, Utils::NotEnoughMemory); 

    class RWStream;
    class StoreRange;

    using boost::scoped_ptr;

    class IWriteProgress : Utils::IBasicInterface
    {
    public:
        virtual void WritePercent(int prog) = 0;
    };

    class FlashIStream
    {          
    public:        

        FlashIStream(Utils::SafeRef<SectorStorageDeviceBase> flash, const StoreRange& range);
        ~FlashIStream();

        Utils::IBinaryReader& getReader();

        // Jump forward to minSkipBytes with alignment to sector begin
        void SectorSeek(dword minSkipBytes); // can throw NoFreeSectors
        
        dword getOffset() const;
        
    // stream semantic for Reader
    private:
        friend class Utils::BinaryReader<FlashIStream>;
        void Read(void *pDest, size_t length); // can throw OutOfRange; 
        byte ReadByte();

    private:       
        scoped_ptr<RWStream> m_streamBase;
        Utils::BinaryReader<FlashIStream> m_reader;

    };

    // ------------------------------------------------------------------------------------

    class FlashOStream
    {            
    public:        

        FlashOStream(Utils::SafeRef<SectorStorageDeviceBase> flash, const StoreRange& range);
        ~FlashOStream();

        Utils::IBinaryWriter& getWriter() { return m_writer; }

        // Jump forward to minSkipBytes with alignment to sector begin
        void SectorSeek(dword minSkipBytes); // can throw NoFreeSectors

        // Остаточный размер с учетом выравнивания тек позиции по началу след. сектора.
        int SizeBeyondCurrPos() const;

        void SetWriteProgressInd(Utils::SafeRef<IWriteProgress> progInd);
        
    // Stream semantic for Writer
    private:      
        friend class Utils::BinaryWriter<FlashOStream>;
        void Write(byte b);
        void Write(const void *pSrc, size_t length); // can throw OutOfRange;

    private:         
        scoped_ptr<RWStream> m_streamBase;
        Utils::BinaryWriter<FlashOStream> m_writer;
    };
} // namespace BfBootSrv

#endif
