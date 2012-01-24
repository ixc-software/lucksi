#ifndef __BFFLASHDRIVERTEST__
#define __BFFLASHDRIVERTEST__

#include "BfDev/BfFlashDriver.h"

namespace BfDev
{

    class BfFlashDriverTest
    {
    public:
        // Tests


        // Tests getSectorBounds() and getSectorByOffset() functions
        static void TestSectorBounds(const BfFlashDriver &flashDriver, dword sectorNumber);

        // Tests WriteData() and ReadData() for the specified sector
        static void TestReadWrite   (const BfFlashDriver &flashDriver, dword sectorNumber);

        // Tests EraseSector()
        static void TestErase       (const BfFlashDriver &flashDriver, dword sectorNumber);

        // Tests raw WriteData() and ReadData() for whole flash memory
        static void TestRawReadWrite(const BfFlashDriver &flashDriver);

        // Runs all BfFlashDriver tests
        static void TestFlashDriver(const BfFlashProfile &flashProfile, bool silent = false);


        // Benchmarks


        // Performs a benchmark of EraseSector() function for all available sectors
        // Return value is total time in ms
        static dword BenchmarkErase(const BfFlashProfile &flashProfile);

        // Performs a benchmark of WriteData() function for all available sectors
        // Return value is total time in ms
        static dword BenchmarkWrite(const BfFlashProfile &flashProfile);

        // Performs a benchmark of ReadData() function for all available sectors
        // Return value is total time in ms
        static dword BenchmarkRead(const BfFlashProfile &flashProfile);

        // Performs all benchmarks
        static void BenchmarkAll(std::string &result, const BfFlashProfile &flashProfile);
    };


} // namespace BfDev


#endif
