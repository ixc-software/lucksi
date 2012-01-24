#include "stdafx.h"

#include "ZlibPipeTest.h"

#include "Utils/RawCRC32.h"
#include "Utils/ZlibPipe.h"
#include "Utils/MemWriterStream.h"
#include "Utils/Random.h"
#include "Utils/IntToString.h"
#include "Utils/BinaryWriter.h"

namespace UtilsTests
{
    using namespace Utils;
    using Platform::byte;
    using Platform::dword;

    const int CRndRange = 16;
    const int CDataSize = 32000;            // must be odd
    const int CDeflateBlockSize = 1024;     // must be misaligned with CDataSize

    const int   CPackedSize     = 18630;
    const dword CDataCRC        = 0x96161266;

    // -------------------------------------------------------------

    void GenerateData(std::vector<byte> &data)
    {
        Utils::Random rnd(1);

        data.reserve(CDataSize);

        for(int i = 0; i < CDataSize; ++i) 
        {
            data.push_back( rnd.Next(CRndRange) );
        }
    }

    // -------------------------------------------------------------

    void Deflate(const std::vector<byte> &data, MemWriterDynStream &dataPacked)
    {
        ZlibDeflate def(dataPacked);

        int sizeOut = 0;

        while(sizeOut < data.size())
        {
            int bytes = data.size() - sizeOut;
            if (bytes > CDeflateBlockSize) bytes = CDeflateBlockSize;
            bool finalize = (sizeOut + bytes) == data.size();

            def.Add(&data[sizeOut], bytes, finalize);
            sizeOut += bytes;
        }
    }

    // -------------------------------------------------------------

    void Inflate(MemWriterDynStream &dataPacked, MemWriterDynStream &dataExtracted)
    {
        ZlibInflate inf(dataExtracted);

        
        BinaryWriter<MemWriterDynStream> writer(dataPacked);
        writer.WriteDword(0);  // write +4 extra bytes

        int size = dataPacked.BufferSize();
        int halfSize = size / 2;
        ESS_ASSERT(halfSize * 2 == size);  // size must be odd

        // add first half
        bool finalized;
        int used = inf.Add(dataPacked.GetBuffer(), halfSize, finalized);
        ESS_ASSERT(used == halfSize && !finalized);

        // add second half
        used = inf.Add(&(dataPacked.GetBuffer()[halfSize]), halfSize, finalized);
        ESS_ASSERT((used + 4) == halfSize && finalized);
    }


    // -------------------------------------------------------------

    void ZlibRunTest(bool silentMode)
    {
        using namespace std;

        // make data-in
        std::vector<byte> dataIn;
        GenerateData(dataIn);

        Platform::dword crcDataIn = UpdateCRC32(dataIn);

        // deflate
        MemWriterDynStream dataPacked;
        Deflate(dataIn, dataPacked);

        int packedSize = dataPacked.BufferSize();
        int ratio = (packedSize * 100) / dataIn.size();

        // assert crc and packed size
        ESS_ASSERT(packedSize == CPackedSize);
        ESS_ASSERT(crcDataIn == CDataCRC);

        if (!silentMode)
        {
            cout << "Ratio " << ratio << "%; CRC " << IntToHexString(crcDataIn) << endl; 
        }

        // inflate
        MemWriterDynStream dataExtracted;
        Inflate(dataPacked, dataExtracted);

        // crc
        Platform::dword crcExtracted = UpdateCRC32(dataExtracted.GetBuffer(), dataExtracted.BufferSize());
        ESS_ASSERT(crcDataIn == crcExtracted);
        
        if (!silentMode)
        {
        	cout << "OK" << endl;
        }
        
    }

    
}  // namespace UtilsTests
