#ifndef __HWFIRMWARE__
#define __HWFIRMWARE__

#include "Utils/StringList.h"
#include "Utils/ManagedList.h"
#include "Utils/ZipArchive.h"
#include "Utils/QtIniFile.h"
#include "Utils/StringUtils.h"
#include "Utils/ComparableT.h"

#include "BfBootCore/ScriptCmdList.h"
#include "BfBootCore/BroadcastMsg.h"

#include "BfBootCore/IHwFirmware.h"

namespace IpsiusService
{
    using boost::scoped_ptr;

    // прошивка платы 
    class HwFirmware : boost::noncopyable, public BfBootCore::IHwFirmware
    {

        class HwRangeList;
        class BinImage;
        class BootScript;

    // IHwFirmware impl
    public:

        QString Info() const;

        int Release() const { return m_releaseNumber; }

        int ImageCount() const;
        BfBootCore::IHwFirmwareImage& Image(int index) const;

        int ScriptCount() const;
        BfBootCore::IHwFirmwareScript& Script(int index) const;

    public:

        ESS_TYPEDEF(Error);

        enum
        {
            CMaxImages = 0x10,
        };

        

        HwFirmware(const QFileInfo &fileInfo);        

        ~HwFirmware();       

        int ReleaseCompare(const HwFirmware &other) const
        {
            return Utils::CompareToInt(m_releaseNumber, other.m_releaseNumber);
        }

        int ReleaseCompare(const BfBootCore::BroadcastMsg &msg) const
        {
            int msgSoftVer = msg.SoftRelNumber;
            return Utils::CompareToInt(m_releaseNumber, msgSoftVer);
        }

        // прошивка подходит для платы по HwType + HwNum
        bool Accepted(const BfBootCore::BroadcastMsg &msg) const;

        const QFileInfo& getFileInfo() const;

    private:        

        QFileInfo m_fileInfo;
        Utils::ZipArchive m_zip;

        scoped_ptr<HwRangeList> m_number;
        scoped_ptr<HwRangeList> m_ID;
        int m_releaseNumber;
        Utils::ManagedList<BinImage> m_images;
        Utils::ManagedList<BootScript> m_scripts;

        void ScanImages(const Utils::IniFileSection &sect);
        void VerifyImageIndexes(int count);
        void ScanScripts(const Utils::IniFileSection &sect);

        BootScript* FindScript(const std::string &scriptName);
        void ParseIndex(const Utils::StringList &sl);

    };



}  // namespace IpsiusService





#endif
