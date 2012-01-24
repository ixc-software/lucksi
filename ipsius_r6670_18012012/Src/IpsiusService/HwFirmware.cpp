#include "stdafx.h"

#include "HwFirmware.h"
#include "Utils/RangeList.h"

// -------------------------------------------------

namespace
{
    bool StringToDword(const std::string& s, Platform::dword& result)
    {
        std::istringstream ss;
        ss.str(s);
        ss >> result;
        if (ss.fail()) return false;//ESS_THROW_MSG(IpsiusService::HwFirmware::Error, "");
        else           return true;
    }    
}

namespace IpsiusService
{
    using Platform::dword;

    /*
    class HwFirmware::HwNumber
    {       
        scoped_ptr<Utils::RangeList> m_rangeList;

    public:
        HwNumber(const std::string &s)  // can throw
        {              
            std::string err;
            m_rangeList.reset(Utils::RangeList::Create(s, err));
            if (!m_rangeList) ESS_THROW_MSG(Error, "Can`t read HwNumber " + err);
        }

        bool Accepted(const BfBootCore::BroadcastMsg &msg) const
        {            
            if (m_rangeList->Size() == 0) return true;
            return m_rangeList->ExistValue(msg.HwNumber);
        }
    };

    class HwFirmware::HwID
    {       
        scoped_ptr<Utils::RangeList> m_rangeList;

    public:
        HwID(const std::string &s)  // can throw
        {              
            std::string err;
            m_rangeList.reset(Utils::RangeList::Create(s, err));
            if (!m_rangeList) ESS_THROW_MSG(Error, "Can`t read HwID " + err);
        }

        bool Accepted(const BfBootCore::BroadcastMsg &msg) const
        {            
            return m_rangeList->ExistValue(msg.HwType);
        }
    }; */

    class HwFirmware::HwRangeList
    {       
        scoped_ptr<Utils::RangeList> m_rangeList;

    public:

        HwRangeList(const std::string &s, const std::string &typeDesc, bool allowNull) // can throw
        {              
            std::string err;

            m_rangeList.reset( Utils::RangeList::Create(s, err) );

            if (m_rangeList == 0) 
            {
                ESS_THROW_MSG(Error, "Can`t read " + typeDesc + ": " + err);
            }

            if ((m_rangeList->Size() == 0) && !allowNull)
            {
                ESS_THROW_MSG(Error, "Null set in " + typeDesc);
            }

        }

        bool Accepted(int number) const
        {            
            if (m_rangeList->Size() == 0) return true;  // null works like wildcard
            return m_rangeList->ExistValue(number);
        }
    };

    // ---------------------------------------------

    class HwFirmware::BinImage : public BfBootCore::IHwFirmwareImage
    {
        Utils::ZipArchive &m_zip;  // vs cache body in constructor - ?
        int m_fileSize;
        QByteArray m_body;

        std::string m_fileName;
        std::string m_versionInfo;

        QByteArray ProcessFile(bool withLoad)
        {
            // find in zip
            if (!m_zip.SetCurrent(m_fileName)) 
            {
                std::string msg = "Can't find file in archive : ";
                msg += m_fileName;
                ESS_THROW_MSG(Error, msg);
            }

            // size
            m_fileSize = m_zip.Current().Size();
            if (m_fileSize <= 0) 
            {
                std::string msg = "Bad file size in archive : ";
                msg += m_fileName;
                ESS_THROW_MSG(Error, msg);
            }

            // load
            QByteArray res;

            if (withLoad)
            {
                res.resize(m_fileSize);

                int bytesReaded;
                m_zip.Current().Read(res.data(), res.size(), bytesReaded);
                if (bytesReaded != res.size()) 
                {
                    std::string msg = "Size mismatch : ";
                    msg += m_fileSize;
                    ESS_THROW_MSG(Error, msg);
                }
            }

            return res;
        }

    // IHwFirmwareImage impl
    public:

        QByteArray Data()  // can throw 
        {
            if (m_body.size() == 0)
            {
                m_body = ProcessFile(true);
            }

            return m_body;
        }

        QByteArray Data() const
        {
            return const_cast<BinImage*>(this)->Data(); // -- ?
        }


        std::string Info() const
        {
            return m_versionInfo;
        }

    public:

        BinImage(const std::string &desc, Utils::ZipArchive &zip) :  // can throw
          m_zip(zip)
          {
              char CSep = '/';

              int pos = desc.find(CSep);
              if (pos < 0) 
              {
                  std::string msg = "Can't parse image description: ";
                  msg += desc;
                  ESS_THROW_MSG(Error, msg);
              }

              m_fileName = desc.substr(0, pos);
              m_versionInfo = desc.substr(pos + 1);

              // verify file
              ProcessFile(false);
          }

    };

    // ---------------------------------------------

    class HwFirmware::BootScript : public BfBootCore::IHwFirmwareScript
    {
        std::string m_name; 
        std::string m_value;

    // IHwFirmwareScript impl
    public:

        std::string Name() const  { return m_name; }
        std::string Value() const { return m_value; }

    public:

        BootScript(const std::string &name, const std::string &value) : 
          m_name(name),
              m_value(value)
          {
          }

    };

}  // namespace IpsiusService

// -------------------------------------------------

namespace IpsiusService
{

    QString HwFirmware::Info() const
    {
        QString res;
        
        res += m_fileInfo.fileName();

        // release
        res += QString("; release %1").arg(m_releaseNumber);

        // images
        res += "; images { ";
        for(int i = 0; i < m_images.Size(); ++i)
        {
            res += m_images[i]->Info().c_str();
            if (i != m_images.Size() - 1) res += ", ";
        }
        res += "}";

        // scripts
        res += "; scripts { ";
        for(int i = 0; i < m_scripts.Size(); ++i)
        {
            res += m_scripts[i]->Name().c_str();
            if (i != m_scripts.Size() - 1) res += ", ";
        }
        res += "}";

        return res;
    }

    // -----------------------------------------------------

    int HwFirmware::ImageCount() const
    {
        return m_images.Size();
    }

    // -----------------------------------------------------

    BfBootCore::IHwFirmwareImage& HwFirmware::Image( int index ) const
    {
        BinImage *p = m_images[index];
        ESS_ASSERT(p != 0);
        return *p;
    }

    // -----------------------------------------------------

    int HwFirmware::ScriptCount() const
    {
        return m_scripts.Size();
    }

    // -----------------------------------------------------

    BfBootCore::IHwFirmwareScript& HwFirmware::Script( int index ) const
    {
        return *m_scripts[index];
    }

    // -----------------------------------------------------

    HwFirmware::HwFirmware( const QFileInfo &fileInfo ) : // can throw 
        m_fileInfo(fileInfo),
        m_zip(fileInfo.absoluteFilePath().toStdString()),
        m_releaseNumber(0),
        m_images(true, 0, true)
    {
        m_images.Resize(CMaxImages);

        // load ini file body
        if (!m_zip.SetCurrent("index.ini")) ESS_THROW_MSG(Error, "Can't locate index file!");

        std::vector<Platform::byte> buff;
        m_zip.Current().ReadAll(buff);
        if (buff.empty()) ESS_THROW_MSG(Error, "Index file is empty!");

        Utils::StringList sl(&buff[0], buff.size());
        ParseIndex(sl);
    }

    HwFirmware::~HwFirmware()
    {
        // empty
    }

    // -----------------------------------------------------

    bool HwFirmware::Accepted( const BfBootCore::BroadcastMsg &msg ) const
    {
        ESS_ASSERT(m_number != 0 && m_ID != 0);

        if (m_number->Accepted(msg.HwNumber) && 
            m_ID->Accepted(msg.HwType)) return true;

        return false;
    }

    // -----------------------------------------------------

    void HwFirmware::ScanImages( const Utils::IniFileSection &sect )
    {
        static const std::string CImage = "Image";

        int maxIndex = -1;

        for(int i = 0; i < sect.ItemsCount(); ++i)
        {
            std::string name = sect[i].Name();
            if (!Utils::StringStartWith(name, CImage)) continue;

            std::string num = name.substr(CImage.size());
            int index;
            if ( !Utils::StringToInt(num, index) ) 
            {
                std::string msg = "Bad Image name: ";
                msg += name;
                ESS_THROW_MSG(Error, msg);
            }

            if (index > maxIndex) maxIndex = index;

            // index range
            if (!m_images.IndexInRange(index)) 
            {
                std::string msg = "Bad Image name (index): ";
                msg += name;
                ESS_THROW_MSG(Error, msg);
            }

            ESS_ASSERT(m_images[index] == 0);

            m_images.Set(index, new BinImage(sect.GetItemValue(name), m_zip));
        }

        VerifyImageIndexes(maxIndex + 1);
    }

    // -----------------------------------------------------

    void HwFirmware::VerifyImageIndexes( int count )
    {
        // verify indexes
        for(int i = 0; i < count; ++i)
        {
            if (m_images[i] == 0)
            {
                std::ostringstream oss;
                oss << "Images hole at " << i;
                ESS_THROW_MSG(Error, oss.str());
            }
        }

        // fix size
        m_images.Resize(count);
    }

    // -----------------------------------------------------

    void HwFirmware::ScanScripts( const Utils::IniFileSection &sect )
    {
        static const std::string CScript = "Script";

        for(int i = 0; i < sect.ItemsCount(); ++i)
        {
            std::string name = sect[i].Name();
            if (!Utils::StringStartWith(name, CScript)) continue;

            std::string scriptName = name.substr(CScript.size());
            if (scriptName.empty()) 
            {
                std::string msg = "Empty script name: ";
                msg += name;
                ESS_THROW_MSG(Error, msg);
            }

            if (FindScript(scriptName) != 0)
            {
                std::string msg = "Dublicate script name: ";
                msg += scriptName;
                ESS_THROW_MSG(Error, msg);
            }

            // verify script
            std::string script = sect.GetItemValue(name);
            std::string scriptErr;

            if (!BfBootCore::ScriptCmdList::Validate(script, m_images.Size(), &scriptErr))
            {
                std::ostringstream oss;
                oss << "Script syntax error; script name " << name
                    << "; error " << scriptErr;                        
                ESS_THROW_MSG(Error, oss.str());                   
            }

            m_scripts.Add( new BootScript(scriptName, script) );
        }
    }

    // -----------------------------------------------------

    HwFirmware::BootScript* HwFirmware::FindScript( const std::string &scriptName )
    {
        for(int i = 0; i < m_scripts.Size(); ++i)
        {
            if (m_scripts[i]->Name() == scriptName) return m_scripts[i];
        }

        return 0;
    }

    // -----------------------------------------------------

    void HwFirmware::ParseIndex( const Utils::StringList &sl )
    {
        Utils::IniFileConfig cfg;
        // cfg.AllowDublicateItems = true;

        Utils::QtIniFile ini(sl, cfg);
        const Utils::IniFileSection &sect = ini[0];

        // static config fields
        m_number.reset( new HwRangeList(sect.GetItemValue("HwNumber"), "HwNumber", true) );
        m_ID.reset( new HwRangeList(sect.GetItemValue("HwID"), "HwID", false) );
        m_releaseNumber = sect.Item("Release").AsInt();

        // scan images
        ScanImages(sect);

        // scan scripts 
        ScanScripts(sect);
    }

    // -----------------------------------------------------

    const QFileInfo& HwFirmware::getFileInfo() const
    {
        return m_fileInfo;
    }

}  // namespace IpsiusService

