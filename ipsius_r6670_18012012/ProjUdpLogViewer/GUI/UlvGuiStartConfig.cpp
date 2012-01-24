
#include "stdafx.h"
#include "UlvGuiStartConfig.h"
#include "Core/UlvUtils.h"


namespace
{
    using namespace UlvGui;

    const QByteArray CShowFilterBox = "ShowFilterBox";
    const QByteArray CShowDebugBox = "ShowDebugBox";
    const QByteArray CShowInfoBox = "ShowInfoBox";
    // const QByteArray CEnableUseRcvrTimestamp = "EnableUseReceiverTimestamp";
    // const QByteArray CEnableUseSrcInLog = "EnableUseSourceNameInLog";
    const QByteArray CTimestampModeIndex = "TimestampModeIndex";
    const QByteArray CFormWidth = "Width";
    const QByteArray CFormHeight = "Height";
    const QByteArray CFormTop = "X";
    const QByteArray CFormLeft = "Y";

    // ------------------------------------------------------------------------

    QByteArray FromBool(bool val)
    {
        return (val)? "true" : "false";
    }

    // ------------------------------------------------------------------------

    QByteArray MakeConfig(const StartConfigInfo &info)
    {
        QByteArray res;
        res += "// Set true to show filter box.\n";
        res += CShowFilterBox;
        res += " = ";
        res += FromBool(info.ShowFilterBox);
        res += "\n\n";

        res += "// Set true to show debug box.\n";
        res += CShowDebugBox;
        res += " = ";
        res += FromBool(info.ShowDebugBox);
        res += "\n\n";

        res += "// Set true to show info box (it also displays info about errors).\n";
        res += CShowInfoBox;
        res += " = ";
        res += FromBool(info.ShowInfoBox);
        res += "\n\n";

        /*
        res += "// Set true to enable using receiver side timestamp in log.\n";
        res += CEnableUseRcvrTimestamp;
        res += " = ";
        res += FromBool(info.EnableUseRcvrTimestamp);
        res += "\n\n";

        res += "// Set true to enable using log sources short description in log.\n";
        res += CEnableUseSrcInLog;
        res += " = ";
        res += FromBool(info.EnableUseSrcInLog);
        res += "\n\n";
        */
        
        res += "// How timestamp will be displayed.\n";
        res += CTimestampModeIndex;
        res += " = ";
        res += QByteArray::number(info.TimestampModeIndex);
        res += "\n\n";

        res += "// Main window size (if set size less than minimum, it'll be set to minimum).\n";
        res += CFormWidth;
        res += " = ";
        res += QByteArray::number(info.FormWidth);
        res += "\n\n";
        
        res += CFormHeight;
        res += " = ";
        res += QByteArray::number(info.FormHeight);
        res += "\n\n";

        res += "// Main window position .\n";
        res += CFormTop;
        res += " = ";
        res += QByteArray::number(info.FormTop);
        res += "\n\n";

        res += CFormLeft;
        res += " = ";
        res += QByteArray::number(info.FormLeft);
        res += "\n\n";

        return res;
    }

    // ------------------------------------------------------------------------

    QByteArray DefaultConfig()
    {
        StartConfigInfo info;
        info.ShowFilterBox = false;

        return MakeConfig(info);
    }

} // namespace

// ------------------------------------------------------------------------

namespace UlvGui
{
    // if file doesn't exists or config in file is invalid,
    // it'll create default config file with given name
    StartConfig::StartConfig(const QString &fileName) :
        m_fileName(fileName)
    {
        RegisterMeta();
        LoadConfig();
    }

    // ------------------------------------------------------------------------

    StartConfig::~StartConfig()
    {
        WriteConfigToFile(MakeConfig(m_info));
    }

    // ------------------------------------------------------------------------

    void StartConfig::TimestampModeIndex(int val)
    {
        ESS_ASSERT(val >= 0);
        m_info.TimestampModeIndex = val;        
    }

    // ------------------------------------------------------------------------

    void StartConfig::RegisterMeta()
    {
        // m_config.Add(m_info.EnableUseRcvrTimestamp, CEnableUseRcvrTimestamp, "filter flag");
        // m_config.Add(m_info.EnableUseSrcInLog, CEnableUseSrcInLog, "filter flag");
        m_config.Add(m_info.ShowDebugBox, CShowDebugBox, "form panel");
        m_config.Add(m_info.ShowInfoBox, CShowInfoBox, "form panel");
        m_config.Add(m_info.ShowFilterBox, CShowFilterBox, "form panel");
        m_config.Add(m_info.TimestampModeIndex, CTimestampModeIndex, "timestamp mode index");
        m_config.Add(m_info.FormWidth, CFormWidth, "form width");
        m_config.Add(m_info.FormHeight, CFormHeight, "form height");
        m_config.Add(m_info.FormTop, CFormTop, "form top left x pos");
        m_config.Add(m_info.FormLeft, CFormLeft, "form top left y pos");

        m_config.Complete();
    }

    // ------------------------------------------------------------------------

    void StartConfig::LoadConfig()
    {
        bool loadFailed = false;
        // try load
        if (QFile::exists(m_fileName))
        {
            try
            {
                m_config.LoadFromFile(m_fileName);                
            }
            catch(Utils::MetaConfig::BadConversion &e)
            {
                loadFailed = true;
            }
            catch(Utils::MetaConfig::UnknownFieldName &e)
            {
                loadFailed = true;
            }
            catch(Utils::MetaConfigLoader::DublicateName &e)
            {
                loadFailed = true;
            }
            catch(Utils::MetaConfigLoader::SyntaxError &e)
            {
                loadFailed = true;
            }
            catch(Utils::IStoragable::IoError &e)
            {
                ESS_THROW_MSG(StartConfigError, e.getTextMessage());
            }

            // validate
            if (m_info.TimestampModeIndex < 0) { loadFailed = true; }

            if (!loadFailed) return;
        }

        // write and load default
        WriteConfigToFile(DefaultConfig());
        m_config.LoadFromFile(m_fileName); // load shouldn't fail
    }

    // ------------------------------------------------------------------------

    void StartConfig::WriteConfigToFile(const QByteArray &config)
    {
        QString err;
        UlvUtils::SaveToFile(m_fileName, config, false, err);

        if (err.isEmpty()) return;

        ESS_THROW_MSG(StartConfigError, err.toStdString());
    }

    // ------------------------------------------------------------------------


} // namespace UlvGui
