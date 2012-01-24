

#include "stdafx.h"

#include "UlvStartupParams.h"

#include "Utils/ProgramOptionsSetters.h"
#include "Utils/ProgramOptions.h"

namespace
{
    using namespace Ulv;

    const std::string CFlagDBSize               = "s";          // int
    const std::string CFlagPortNumAndType       = "pt";         // int type
    const std::string CFlagDisableRecording     = "norec";      // none
    const std::string CFlagRecordingIntervalMs  = "rec_int";    // int
    const std::string CFlagRecorgingCount       = "rec_count";  // int

    const std::string CPortDataTypeStr      = "str";
    const std::string CPortDataTypeWstr     = "wstr";

    const QString CSepPortAndType           = ":";
    const QString CSepPorts                 = ";";


    // ---------------------------------------------------------------

    std::vector<std::string> Flags(const std::string &flag)
    {
        std::vector<std::string> res;
        res.push_back(flag);
        return res;
    }

    // ---------------------------------------------------------------

    void InvalidCmdLineParamErr(const std::string &param)
    {
        std::string err("Invalid command line parameter: ");
        err += param;
        err += "\n";

        ESS_THROW_MSG(StartupParams::CommandLineError, err);
    }

    // ---------------------------------------------------------------

    void ToFewCmdLineParamsErr()
    {
        ESS_THROW_MSG(StartupParams::CommandLineError, "To few command line parameters");
    }

    // ---------------------------------------------------------------

    UdpPortDataType ToPortType(const std::string &str)
    {
        if (str == CPortDataTypeStr) return PTString;
        if (str == CPortDataTypeWstr) return PTWstring;

        InvalidCmdLineParamErr(str);

        return PTString; // never happend
    }
}

// ---------------------------------------------------------------

namespace Ulv
{
    StartupParams::StartupParams() :
        m_isValid(false)
    {
    }

    // ---------------------------------------------------------------

    void StartupParams::SavePortAndType(const std::vector<std::string> &params) // can throw
    {
        if (params.empty()) ToFewCmdLineParamsErr();

        for (size_t i = 0; i < params.size(); ++i)
        {
            QString p(params.at(i).c_str());
            QStringList portAndType = p.split(CSepPortAndType);
            if (portAndType.size() != 2) InvalidCmdLineParamErr(params.at(i));

            std::string portName = portAndType.at(0).trimmed().toStdString();
            std::string portType = portAndType.at(1).trimmed().toStdString();
            int port = 0;
            if ((!Utils::StringToInt(portName, port)) || (port < 0))
            {
                InvalidCmdLineParamErr(portName);
            }

            UdpPortDataType type = ToPortType(portType); // can throw

            m_ports.push_back(PortAndType(port, type));
        }
    }

    // ---------------------------------------------------------------

    void StartupParams::SaveRecordCount(const std::string &param)
    {
        int recCount = 0;

        if (!Utils::StringToInt(param, recCount) || (recCount <= 0))
        {
            InvalidCmdLineParamErr(param);
        }

        m_dbParams.RecordCount(recCount);
    }

    // ---------------------------------------------------------------

    void StartupParams::SaveDisableRecording()
    {
        m_dbParams.EnableRecording(false);
    }

    // ---------------------------------------------------------------

    void StartupParams::SaveRecordingIntervalMs(const std::string &param) // can throw
    {
        int interval = 0;

        if (!Utils::StringToInt(param, interval) || (interval < 1))
        {
            InvalidCmdLineParamErr(param);
        }

        m_dbParams.RecordingIntervalMs(interval);
    }

    // ---------------------------------------------------------------

    void StartupParams::SaveRecordingCount(const std::string &param) // can throw
    {
        int count = 0;

        if (!Utils::StringToInt(param, count) || (count < 1))
        {
            InvalidCmdLineParamErr(param);
        }

        m_dbParams.RecordingRecordCount(count);

    }

    // ---------------------------------------------------------------

    void StartupParams::Parse(const Utils::StringList &args)
    {
        using namespace Utils;
        ProgramOptions<T> opt(*this);

        {
            opt.Add(&T::SaveDisableRecording, Flags(CFlagDisableRecording),
                    "disable runtime saving log to file");
        }        
        {
            std::ostringstream desc;
            desc << "timer interval in miliseconds for recording to file \n(default: "
                    << UdpLogDBParams::DefaultRecordingIntervalMs() << ")";
            opt.Add(&T::SaveRecordingIntervalMs, Flags(CFlagRecordingIntervalMs), desc.str());
        }
        {
            std::ostringstream desc;
            desc << "recording in file each N record\n(default: "
                    << UdpLogDBParams::DefaultRecordingRecordCount() << ")";
            opt.Add(&T::SaveRecordingCount, Flags(CFlagRecorgingCount), desc.str());
        }
        {
            std::ostringstream desc;
            desc << "UDP packets DB records count (default: "
                    << UdpLogDBParams::DefaultRecordCount() << ")";
            opt.Add(&T::SaveRecordCount, Flags(CFlagDBSize), desc.str());
        }
        {
            std::ostringstream desc;
            desc << "list of port numbers and received data types(" << CPortDataTypeStr
                    << ", " << CPortDataTypeWstr << "): \nport1:type1[;port2:type2[; ...]]";
            opt.Add(&T::SavePortAndType, Flags(CFlagPortNumAndType), desc.str());
        }
        opt.AddDefaultHelpOption();

        try
        {
            opt.Process(args);
        }
        catch (ProgramOptionsExceptions::Base &e)
        {
            ESS_THROW_MSG(CommandLineError, e.getTextMessage());
        }

        // help
        if (opt.HelpRequested()) ESS_THROW_MSG(HelpRequested, opt.HelpInfo());

        m_isValid = true;
    }

    // ---------------------------------------------------------------

    int StartupParams::Port(size_t index) const
    {
        CheckIfValid();
        ESS_ASSERT(index < m_ports.size());
        return m_ports.at(index).Port;
    }

    // ---------------------------------------------------------------

    UdpPortDataType StartupParams::PortDataType(size_t index) const
    {
        CheckIfValid();
        ESS_ASSERT(index < m_ports.size());
        return m_ports.at(index).Type;
    }

    // ---------------------------------------------------------------

    size_t StartupParams::PortsCount() const
    {
        CheckIfValid();
        return m_ports.size();
    }

    // ---------------------------------------------------------------

    const UdpLogDBParams& StartupParams::DBParams() const
    {
        CheckIfValid();
        return m_dbParams;
    }


} // namespace Ulv

