
#include "stdafx.h"
#include "SafeBiProtoTests.h"

#include "SafeBiProto/SbpUtils.h"
#include "Utils/MemWriterStream.h"
#include "Utils/BinaryWriter.h"

namespace
{
    using namespace SBPUtils;

    void DynamicLengthParserAndWriterTest(dword len, byte checkByteCounter)
    {
        // construct dynamic length
        Utils::MemWriterDynStream stream(0);
        Utils::BinaryWriter<Utils::MemWriterDynStream> writer(stream);
        DynamicLengthWriter(writer, len);

        // parse
        DynamicLengthParser parser;

        size_t byteCounter;
        for (byteCounter = 0; byteCounter < stream.BufferSize(); ++byteCounter)
        {
            if (parser.IsComplete()) break;
            
            byte lenByte = *(stream.GetBuffer() + byteCounter);
            parser.Add(lenByte);
        }

        TUT_ASSERT(byteCounter == checkByteCounter);
        TUT_ASSERT(parser.getLength() == len);

    }

    // ----------------------------------------------

    void DynamicLengthParserInfTest()
    {
        byte b = 5;
        DynamicLengthParser parser;
        
        parser.Add(b);
        TUT_ASSERT(parser.IsComplete());
        TUT_ASSERT(parser.getLength() == b);

        parser.Reset();
        TUT_ASSERT(!parser.IsComplete());
        
        parser.Add(b + 1);
        TUT_ASSERT(parser.IsComplete());
        TUT_ASSERT(parser.getLength() == b + 1);
    }

   
} // namespace

// ----------------------------------------------

namespace SBPTests
{
    using namespace SBPUtils;

    void SbpUtilsTests(bool silentMode)
    {
        DynamicLengthParserAndWriterTest(1, 1); // 00000001
        DynamicLengthParserAndWriterTest(3, 1); // 00000011
        DynamicLengthParserAndWriterTest(16, 1); // 00010000
        DynamicLengthParserAndWriterTest(128, 2); // 10000000 -- coded in 2 bytes of 7 bits
        DynamicLengthParserAndWriterTest(192, 2); // 11000000
        DynamicLengthParserAndWriterTest(32768, 3); // 10000000 00000000
        DynamicLengthParserAndWriterTest(12596344, 4); // 11000000 00110100 01111000
        DynamicLengthParserAndWriterTest(152000000, 4); // 1001 00001111 01010110 00000000

        DynamicLengthParserInfTest();
        
        if (silentMode) return;
        std::cout << "SbpUtils tests okay." << std::endl;
    }
    
} // namespace SBPTests


