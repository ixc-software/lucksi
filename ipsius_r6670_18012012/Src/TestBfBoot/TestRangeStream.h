#ifndef TESTRANGESTREAM_H
#define TESTRANGESTREAM_H

namespace BfBootSrv
{
    class RangeIOBase;
    class SectorStorageDeviceBase;
} // namespace BfBootSrv

namespace TestBfBoot
{
    void TestRangeStream();
    void TestRangeStream(BfBootSrv::SectorStorageDeviceBase& sectorDev, BfBootSrv::RangeIOBase& rangeDev);
} // namespace TestBfBoot

#endif
