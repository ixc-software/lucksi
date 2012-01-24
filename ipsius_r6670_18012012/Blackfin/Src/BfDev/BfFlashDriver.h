#ifndef __FLASHDRIVER__
#define __FLASHDRIVER__

#include "stdafx.h"

#include "Platform/PlatformTypes.h"
#include "Utils/ErrorsSubsystem.h"


namespace BfDev
{

    using Platform::byte;
    using Platform::word;
    using Platform::dword;


    //-------------------------------------------------------------------------


    struct SectorLocation
    {
        SectorLocation() : sectorNumber(0), startOffset(0), endOffset(0) {}

        dword sectorNumber;     // Number of represented sector
        dword startOffset;      // Start offset from the base address
        dword endOffset;        // End offset from the base address

        dword getLength() { return endOffset - startOffset + 1; }
    };


    //-------------------------------------------------------------------------


    struct FlashInfo
    {
        FlashInfo() : manufacturerCode(0), deviceCode(0) {}
        dword       manufacturerCode;
        dword       deviceCode;
        std::string manufacturerName;
        std::string deviceTitle;
        std::string partDescription;
    };


    //-------------------------------------------------------------------------


    class BfFlashProfile
    {
        dword m_flashStartAddress;
        void *m_pEntryPoint;

        BfFlashProfile(dword flashStartAddress, void *pEntryPoint) :
        m_flashStartAddress(flashStartAddress),
            m_pEntryPoint(pEntryPoint)
        {
            ESS_ASSERT(pEntryPoint != 0);
        }

    public:

        dword getFlashStartAddress() const { return m_flashStartAddress; }
        void* getEntryPoint() const        { return m_pEntryPoint; }

        static BfFlashProfile M29W320();
    };


    //-------------------------------------------------------------------------


    class BfFlashDriver : boost::noncopyable
    {
        const BfFlashProfile m_flashProfile;

        bool m_exceptionsEnabled;
        bool m_driverInitialized;

        std::vector<SectorLocation> m_sectorMap;

        // Flash info (manufacturer and device codes)
        FlashInfo m_flashInfo;

        bool RequestFlashInfo();
        bool RequestSectorMap();

        void EnableFlashMode() const;

        bool IsRangeCorrect(dword startOffset, dword dataLength) const;


    public:
        // Base exception class
        ESS_TYPEDEF(BfFlashDriverException);

        // Child exception classes derived from BfFlashDriverException class
        ESS_TYPEDEF_FULL(InitializationFailed, BfFlashDriverException);
        ESS_TYPEDEF_FULL(WriteError,           BfFlashDriverException);
        ESS_TYPEDEF_FULL(SendCommandError,     BfFlashDriverException);


        BfFlashDriver(const BfFlashProfile &flashProfile, bool enableExceptions = true);
        ~BfFlashDriver();

        bool IsInitialized() const;

        dword          getSectorsNumber() const;
        SectorLocation getSectorBounds(dword sectorNumber) const;
        SectorLocation getSectorByOffset(dword offset) const;
        FlashInfo      getFlashInfo() const;
        dword          getFlashStartAddress() const;

        void printSectorsInfoTo(std::string &sectorMap);

        bool EraseSector(dword sectorNumber) const;
        bool EraseAll() const;

        // dataLength must be even
        void ReadData (dword startOffset, std::vector<byte> &data, dword dataLength) const;

        // dataLength must be even
        void ReadData (dword startOffset, void *data, dword dataLength) const;

        // startOffset and dataLength must be even
        bool WriteData(dword startOffset, const std::vector<byte> &data, bool verify) const;

        // startOffset and dataLength have must be even
        bool WriteData(dword startOffset, const void * const data, dword dataLength, bool verify) const;
    };

} // namespace BfDev


#endif
