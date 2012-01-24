#ifndef __ICOREMSGBENCHMARKS__
#define __ICOREMSGBENCHMARKS__

namespace iCoreTests
{
    std::string iCoreBenchmarkSingle(int msTimeToRun);
	std::string iCoreBenchmarkSingleTwoThread(int msTimeToRun);
	std::string iCoreBenchmarkMulti(int msTimeToRun);

    std::string iCoreBenchmarkSingleN(int msTimeToRun, int threadsCount);

} // namespace iCoreTests

#endif



