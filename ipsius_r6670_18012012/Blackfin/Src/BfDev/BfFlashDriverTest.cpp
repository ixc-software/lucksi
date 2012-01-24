#include "stdafx.h"

#include "BfDev/BfFlashDriverTest.h"
#include "Platform/Platform.h"

namespace BfDev
{

    using namespace Platform;
    using namespace std;

    void BfFlashDriverTest::TestSectorBounds(const BfFlashDriver &flashDriver, dword sectorNumber)
    {
        SectorLocation sectorBounds = flashDriver.getSectorBounds(sectorNumber);
        TUT_ASSERT(sectorBounds.sectorNumber == sectorNumber);

        TUT_ASSERT(flashDriver.getSectorByOffset(sectorBounds.startOffset).sectorNumber == sectorNumber);
        TUT_ASSERT(flashDriver.getSectorByOffset(sectorBounds.endOffset).sectorNumber == sectorNumber);

        if (sectorNumber > 0)
            TUT_ASSERT(flashDriver.getSectorByOffset(sectorBounds.startOffset-1).sectorNumber == sectorNumber-1);

        if (sectorNumber < flashDriver.getSectorsNumber() - 1)
            TUT_ASSERT(flashDriver.getSectorByOffset(sectorBounds.endOffset+1).sectorNumber == sectorNumber+1);
    }


    //-------------------------------------------------------------------------


    void BfFlashDriverTest::TestReadWrite(const BfFlashDriver &flashDriver, dword sectorNumber)
    {
        const dword CWriteBufferLength = 128;
        const int   CRandomSeed        = 0;

        SectorLocation sectorBounds   = flashDriver.getSectorBounds(sectorNumber);
        dword          sectorLength   = sectorBounds.getLength();
        dword          numberOfBlocks = sectorLength / CWriteBufferLength;

        ESS_ASSERT((sectorLength % CWriteBufferLength) == 0);


        // Write (numberOfBlocks) blocks
        srand(CRandomSeed);
        dword currentOffset = sectorBounds.startOffset;
        for (dword i = 0; i < numberOfBlocks; ++i, currentOffset += CWriteBufferLength)
        {
            // Prepare a write buffer
            vector<byte> buffer;
            for (dword j = 0; j < CWriteBufferLength; ++j)
                buffer.push_back( static_cast<byte>(rand()) );

            // Write data
            flashDriver.WriteData(currentOffset, buffer, true);
        }


        // Verify
        srand(CRandomSeed);
        currentOffset = sectorBounds.startOffset;
        for (dword i = 0; i < numberOfBlocks; ++i, currentOffset += CWriteBufferLength)
        {
            // Read data
            vector<byte> buffer;
            flashDriver.ReadData(currentOffset, buffer, CWriteBufferLength);

            // Compare
            for (dword j = 0; j < CWriteBufferLength; ++j)
            {
                byte test = static_cast<byte>(rand());
                TUT_ASSERT( buffer[j] == test );
            }
        }
    }


    //-------------------------------------------------------------------------


    void BfFlashDriverTest::TestErase(const BfFlashDriver &flashDriver, dword sectorNumber)
    {
        SectorLocation sectorBounds = flashDriver.getSectorBounds(sectorNumber);

        word haveToBeFFFF = 0xFFFF;
        word *pData = reinterpret_cast<word*>(flashDriver.getFlashStartAddress() + sectorBounds.startOffset);

        // Erase sector
        flashDriver.EraseSector(sectorNumber);

        // Verify
        dword i = sectorBounds.getLength() >> 1;
        while (i--)
            TUT_ASSERT(*pData++ == haveToBeFFFF);
    }


    //-------------------------------------------------------------------------


    void BfFlashDriverTest::TestRawReadWrite(const BfFlashDriver &flashDriver)
    {
        const dword CRawBufferLength = 128;
        const int   CRandomSeed      = 0;

        dword flashLength    = flashDriver.getSectorBounds(flashDriver.getSectorsNumber() - 1).endOffset + 1;
        dword numberOfBlocks = flashLength / CRawBufferLength;

        ESS_ASSERT(flashLength % CRawBufferLength == 0);


        // Write data
        srand(CRandomSeed);
        for (dword i = 0, offset = 0; i < numberOfBlocks; ++i, offset += CRawBufferLength)
        {
            // Prepare raw buffer
            vector<byte> rawBuffer;
            for (dword i = 0; i < CRawBufferLength; ++i)
                rawBuffer.push_back( static_cast<byte>(rand()) );

            // Write
            flashDriver.WriteData(offset, rawBuffer, true);
        }


        // Verify
        srand(CRandomSeed);
        for (dword i = 0, offset = 0; i < numberOfBlocks; ++i, offset += CRawBufferLength)
        {
            // Read data
            vector<byte> rawBuffer;
            flashDriver.ReadData(offset, rawBuffer, CRawBufferLength);

            // Compare
            for (dword j = 0; j < CRawBufferLength; ++j)
            {
                TUT_ASSERT( rawBuffer[j] == static_cast<byte>(rand()) );
            }
        }
    }


    //-------------------------------------------------------------------------


    void BfFlashDriverTest::TestFlashDriver(const BfFlashProfile &flashProfile, bool silent)
    {
        BfFlashDriver flashDriver(flashProfile);

        if (!silent) cout << "BfFlashDriver::Test() - Preparing..." << endl;

        // Erase flash
        flashDriver.EraseAll();

        if (!silent) cout << "BfFlashDriver::Test() - Starting tests..." << endl;


        // Read, write, and erase tests for each sector
        dword sectorsNumber = flashDriver.getSectorsNumber();
        for (dword sectorNumber = 0; sectorNumber < sectorsNumber; ++sectorNumber)
        {
            // Test getSectorBounds() and getSectorByOffset() functions
            TestSectorBounds(flashDriver, sectorNumber);

            // Write test (with verify = true) and Read test
            TestReadWrite(flashDriver, sectorNumber);

            // Test EraseSector()
            TestErase(flashDriver, sectorNumber);

            if (!silent) cout << "Sector #" << sectorNumber << " passed" << std::endl;
        }


        if (!silent)
        {
            cout
                << "BfFlashDriver::Test() - Test for each sector completed"
                << endl << endl
                << "BfFlashDriver::Test() - Starting raw read/write tests for whole flash..."
                << endl;
        }

        TestRawReadWrite(flashDriver);

        if (!silent) cout << "BfFlashDriver::Test() - Test competed" << endl;
    }


    //-------------------------------------------------------------------------


    dword BfFlashDriverTest::BenchmarkErase(const BfFlashProfile &flashProfile)
    {
        BfFlashDriver flashDriver(flashProfile);
        dword sectorsNumber = flashDriver.getSectorsNumber();

        dword startTime = GetSystemTickCount();
        for (dword sector = 0; sector < sectorsNumber; ++sector)
        {
            flashDriver.EraseSector(sector);
        }

        return GetSystemTickCount() - startTime;
    }


    //-------------------------------------------------------------------------


    dword BfFlashDriverTest::BenchmarkWrite(const BfFlashProfile &flashProfile)
    {
        const dword CBufferLength = 128;

        BfFlashDriver flashDriver(flashProfile);
        dword sectorsNumber = flashDriver.getSectorsNumber();


        // Prepare buffer
        vector<byte> data;
        for (dword i = 0; i < CBufferLength; ++i) data.push_back(i);


        // Write data
        dword startTime = GetSystemTickCount();
        for (dword secNum = 0; secNum < sectorsNumber; ++secNum)
        {
            SectorLocation sector = flashDriver.getSectorBounds(secNum);
            dword numberOfBlocks = sector.getLength() / CBufferLength;

            ESS_ASSERT((sector.getLength() % CBufferLength) == 0);

            dword block  = numberOfBlocks;
            dword offset = sector.startOffset;
            while (--block) 
            {
                flashDriver.WriteData(offset, data, false);
                offset += CBufferLength;
            }
        }

        return GetSystemTickCount() - startTime;
    }


    //-------------------------------------------------------------------------


    dword BfFlashDriverTest::BenchmarkRead(const BfFlashProfile &flashProfile)
    {
        const dword CBufferLength = 128;

        BfFlashDriver flashDriver(flashProfile);
        dword sectorsNumber = flashDriver.getSectorsNumber();


        // Read data
        dword startTime = GetSystemTickCount();
        for (dword secNum = 0; secNum < sectorsNumber; ++secNum)
        {
            SectorLocation sector = flashDriver.getSectorBounds(secNum);
            dword numberOfBlocks = sector.getLength() / CBufferLength;

            ESS_ASSERT((sector.getLength() % CBufferLength) == 0);

            vector<byte> data;

            dword block  = numberOfBlocks;
            dword offset = sector.startOffset;
            while (--block)
            {
                flashDriver.ReadData(offset, data, CBufferLength);
                offset += CBufferLength;
            }
        }

        return GetSystemTickCount() - startTime;
    }


    //-------------------------------------------------------------------------


    void BfFlashDriverTest::BenchmarkAll(string &result, const BfFlashProfile &flashProfile)
    {
        ostringstream ss;
        ss
            << "EraseSector() for all sectors total time: " << BenchmarkErase(flashProfile) << std::endl
            << "WriteData()   for all sectors total time: " << BenchmarkWrite(flashProfile) << std::endl
            << "ReadData()    for all sectors total time: " << BenchmarkRead(flashProfile);

        result = ss.str();
    }

} // namespace BfDev
