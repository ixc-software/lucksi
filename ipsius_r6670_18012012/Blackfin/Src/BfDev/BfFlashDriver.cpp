//-----------------------------------------------------------------------------
// Blackfin Flash Driver
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "Platform/Platform.h"
#include "BfFlashDriver.h"

#include "drivers/flash/Errors.h"
#include "drivers/flash/util.h"

// Includes for BfFlashProfile class
#include "drivers/adi_dev.h"
#include "drivers/flash/adi_m29w320.h"  // M29W320



namespace BfDev
{
    using namespace Platform;
    using namespace std;


    //-------------------------------------------------------------------------


    BfFlashProfile BfFlashProfile::M29W320()
    {
        BfFlashProfile profile(0x20000000, &ADIM29W320EntryPoint);
        return profile;
    }


	//-------------------------------------------------------------------------


    namespace
    {
        bool GFlashDriverInstanceCreated = false;


        //---------------------------------------------------------------------


        bool SendCommand(void *pEntryPoint, enCntrlCmds command, COMMAND_STRUCT &commandStruct, bool exceptionsEnabled)
        {
            ADI_DEV_PDD_ENTRY_POINT *entry = reinterpret_cast<ADI_DEV_PDD_ENTRY_POINT*>(pEntryPoint);
            if (entry->adi_pdd_Control(0, command, static_cast<void*>(&commandStruct)) == NO_ERR)
                return true;

            if (exceptionsEnabled) ESS_THROW_MSG(BfFlashDriver::SendCommandError, "SendCommand() failed");
            return false;
        }


        //---------------------------------------------------------------------


        ERROR_CODE WriteWord(void *pEntryPoint, word *pAddress, word data)
        {
            ADI_DEV_1D_BUFFER   writeBuffer;
            dword address = IntPtrT<dword>(pAddress).Value();

            // Fill the write buffer structure
            writeBuffer.Data            = static_cast<void*>(&data);
            writeBuffer.pAdditionalInfo = static_cast<void*>(&address);
            writeBuffer.pNext           = 0;


            // Write one word
            ADI_DEV_PDD_ENTRY_POINT *entry  = reinterpret_cast<ADI_DEV_PDD_ENTRY_POINT*>(pEntryPoint);
            ADI_DEV_BUFFER          *buffer = reinterpret_cast<ADI_DEV_BUFFER*>(&writeBuffer);
            ERROR_CODE              result  = static_cast<ERROR_CODE>( entry->adi_pdd_Write(0, ADI_DEV_1D, buffer) );
            return result;
        }

    }


    //-------------------------------------------------------------------------


    BfFlashDriver::BfFlashDriver(const BfFlashProfile &flashProfile, bool enableExceptions) :
        m_flashProfile(flashProfile),
        m_exceptionsEnabled(enableExceptions)
	{
        ESS_ASSERT(GFlashDriverInstanceCreated == 0);

        *pEBIU_AMGCTL = 0xFF;

        // Requests flash info (device and manufacturer codes, names, etc.)
        if ((!RequestFlashInfo() || !RequestSectorMap()) && (enableExceptions))
            ESS_THROW_MSG(InitializationFailed, "Flash Driver initialization failed");

        GFlashDriverInstanceCreated = true;
        m_driverInitialized = true;
	}


    //-------------------------------------------------------------------------


	BfFlashDriver::~BfFlashDriver()
	{
        GFlashDriverInstanceCreated = false;
	}


    //-------------------------------------------------------------------------


    bool BfFlashDriver::IsInitialized() const
    {
        return m_driverInitialized;
    }


    //-------------------------------------------------------------------------


    bool BfFlashDriver::RequestFlashInfo()
    {
        COMMAND_STRUCT commandStruct;

        // Setup code so that flash programmer can just read memory instead of call GetCodes()
        commandStruct.SGetCodes.pManCode = reinterpret_cast<unsigned long*>(&m_flashInfo.manufacturerCode);
        commandStruct.SGetCodes.pDevCode = reinterpret_cast<unsigned long*>(&m_flashInfo.deviceCode);
        commandStruct.SGetCodes.ulFlashStartAddr = m_flashProfile.getFlashStartAddress();

        // Get manufacturer and device codes
        if ( !SendCommand(m_flashProfile.getEntryPoint(), CNTRL_GET_CODES, commandStruct, m_exceptionsEnabled) )
            return false;

        // Get information in text
        if ( !SendCommand(m_flashProfile.getEntryPoint(), CNTRL_GET_DESC, commandStruct, m_exceptionsEnabled) )
            return false;

        m_flashInfo.deviceTitle      = commandStruct.SGetDesc.pTitle;
        m_flashInfo.partDescription  = commandStruct.SGetDesc.pDesc;
        m_flashInfo.manufacturerName = commandStruct.SGetDesc.pFlashCompany;

        return true;
    }


    //-------------------------------------------------------------------------


    bool BfFlashDriver::RequestSectorMap()
    {
        // Request number of sectors
        COMMAND_STRUCT commandStruct;
        dword sectorsNumber;

        commandStruct.SGetNumSectors.pnNumSectors = reinterpret_cast<int*>(&sectorsNumber);

        if ( !SendCommand(m_flashProfile.getEntryPoint(), CNTRL_GETNUM_SECTORS, commandStruct, m_exceptionsEnabled) )
            return false;


        // Request sector map
        m_sectorMap.clear();
        for (dword i = 0; i < sectorsNumber; ++i)
        {
            SectorLocation sector;

            commandStruct.SSectStartEnd.nSectorNum   = sector.sectorNumber = i;
            commandStruct.SSectStartEnd.pStartOffset = reinterpret_cast<unsigned long*>(&sector.startOffset);
            commandStruct.SSectStartEnd.pEndOffset   = reinterpret_cast<unsigned long*>(&sector.endOffset);

            if ( !SendCommand(m_flashProfile.getEntryPoint(), CNTRL_GET_SECSTARTEND, commandStruct, m_exceptionsEnabled) )
                return false;

            // Decrement end offset of the last sector
            // (а надо так - черти из AD почему-то его инкрементируют,
            // хотя в документации по чипу ничего такого нет)
            if (i == sectorsNumber - 1) --sector.endOffset;

            // Store sector information
            m_sectorMap.push_back(sector);
        }

        return true;
    }


    //-------------------------------------------------------------------------


    bool BfFlashDriver::IsRangeCorrect(dword startOffset, dword dataLength) const
    {
        dword lastSectorEnd = m_sectorMap[getSectorsNumber() - 1].endOffset;

        if ((startOffset >= lastSectorEnd) || (startOffset + (dataLength - 1) > lastSectorEnd))
            return false;

        return true;
    }


    //-------------------------------------------------------------------------


    dword BfFlashDriver::getSectorsNumber() const
    {
        ESS_ASSERT(m_driverInitialized != 0);

        return m_sectorMap.size();
    }


    //-------------------------------------------------------------------------


    SectorLocation BfFlashDriver::getSectorBounds(dword sectorNumber) const
    {
        ESS_ASSERT(m_driverInitialized != 0);
        ESS_ASSERT(sectorNumber < getSectorsNumber());

        return m_sectorMap[sectorNumber];
    }


    //-------------------------------------------------------------------------


    SectorLocation BfFlashDriver::getSectorByOffset(dword offset) const
    {
        ESS_ASSERT(m_driverInitialized != 0);

        typedef vector<SectorLocation>::const_iterator Iterator;

        // Validate offset
        ESS_ASSERT(offset <= m_sectorMap[getSectorsNumber()-1].endOffset);

        SectorLocation sector;

        for (Iterator it = m_sectorMap.begin(); it != m_sectorMap.end(); ++it)
        {
            sector = *it;

            if ((offset >= sector.startOffset) && (offset <= sector.endOffset))
            {
                return sector;
            }
        }


        ESS_ASSERT(false && "Sector not found, but the offset is valid!");
        return sector;
    }


    //-------------------------------------------------------------------------


    FlashInfo BfFlashDriver::getFlashInfo() const
    {
        ESS_ASSERT(m_driverInitialized != 0);

        return m_flashInfo;
    }


    //-------------------------------------------------------------------------


    dword BfFlashDriver::getFlashStartAddress() const
    {
        ESS_ASSERT(m_driverInitialized != 0);

        return m_flashProfile.getFlashStartAddress();
    }


    //-------------------------------------------------------------------------


    void BfFlashDriver::printSectorsInfoTo(std::string &sectorMap)
    {
        ESS_ASSERT(m_driverInitialized != 0);

        ostringstream ss;

        dword sectorsNumber = m_sectorMap.size();
        for (dword i = 0; i < sectorsNumber; ++i)
        {
            SectorLocation sector = getSectorBounds(i);

            ss
                << "Sector #" << dec << i << ":" << endl
                << "  startOffset:  0x" << hex << sector.startOffset << endl
                << "  endOffset:    0x" << hex << sector.endOffset << endl
                << "  sectorLength: 0x" << hex << sector.getLength() << endl << endl;
        }

        sectorMap = ss.str();
    }


    //-------------------------------------------------------------------------


    bool BfFlashDriver::EraseSector(dword sectorNumber) const
    {
        ESS_ASSERT(m_driverInitialized != 0);

        // Validate sector number
        ESS_ASSERT(sectorNumber < getSectorsNumber());

        COMMAND_STRUCT commandStruct;
        commandStruct.SEraseSect.nSectorNum       = static_cast<int>(sectorNumber);
        commandStruct.SEraseSect.ulFlashStartAddr = m_flashProfile.getFlashStartAddress();

        if ( !SendCommand(m_flashProfile.getEntryPoint(), CNTRL_ERASE_SECT, commandStruct, m_exceptionsEnabled) )
            return false;

        return true;
    }


    //-------------------------------------------------------------------------


    bool BfFlashDriver::EraseAll() const
    {
        ESS_ASSERT(m_driverInitialized != 0);

        COMMAND_STRUCT commandStruct;
        commandStruct.SEraseAll.ulFlashStartAddr = m_flashProfile.getFlashStartAddress();

        if ( !SendCommand(m_flashProfile.getEntryPoint(), CNTRL_ERASE_ALL, commandStruct, m_exceptionsEnabled) )
            return false;

        return true;
    }


    //-------------------------------------------------------------------------


    void BfFlashDriver::ReadData(dword startOffset, vector<byte> &data, dword dataLength) const
    {
        // Reserve (dataLength) elements in a vector
        data.resize(dataLength);

        ReadData(startOffset, &data[0], dataLength);
    }


    //-------------------------------------------------------------------------


    bool BfFlashDriver::WriteData(dword startOffset, const vector<byte> &data, bool verify) const
    {
        ESS_ASSERT(m_driverInitialized != 0);

        // Write data stored in the vector as a raw data
        return WriteData(startOffset, &data[0], data.size(), true);
    }


    //-------------------------------------------------------------------------


    void BfFlashDriver::ReadData(dword startOffset, void *data, dword dataLength) const
    {
        ESS_ASSERT(m_driverInitialized != 0);

        // Check address range
        ESS_ASSERT(IsRangeCorrect(startOffset, dataLength));

        // Read data to the buffer
        IntPtr source(m_flashProfile.getFlashStartAddress() + startOffset);
        IntPtr destination(data);

        // Read data from flash memory
        memcpy(destination.ToVoid(), source.ToVoid(), dataLength);
    }


    //-------------------------------------------------------------------------


    bool BfFlashDriver::WriteData(dword startOffset, const void * const data, dword dataLength, bool verify) const
    {
        ESS_ASSERT(m_driverInitialized != 0);

        // Check offset
        ESS_ASSERT((startOffset & 1) == 0);
        ESS_ASSERT((dataLength & 1) == 0);

        // Check address range
        ESS_ASSERT(IsRangeCorrect(startOffset, dataLength));

        // Write the buffer
        dword absoluteAddress = m_flashProfile.getFlashStartAddress() + startOffset;
        const word *pSource   = reinterpret_cast<const word*>(data);
        word *pDestination    = IntPtr(absoluteAddress).ToPtr<word>();

        dataLength >>= 1;
        for (dword i = 0; i < dataLength; ++i, ++pDestination, ++pSource)
        {
            // Write value to the flash memory
            if (WriteWord(m_flashProfile.getEntryPoint(), pDestination, *pSource) != NO_ERR)
            {
                if  (m_exceptionsEnabled) ESS_THROW_MSG(WriteError, "Error occured while writing");
                return false;
            }

            // Validate written data if needed
            if (verify)
            {
                if (*pSource != *pDestination)
                {
                    if (m_exceptionsEnabled) ESS_THROW_MSG(WriteError, "Data verification failed");
                    return false;
                }
            }
        }

        return true;
    }

} // namespace BfDev
