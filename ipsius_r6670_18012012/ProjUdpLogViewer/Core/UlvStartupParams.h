
#ifndef __ULVSTARTUPPARAMS__
#define __ULVSTARTUPPARAMS__

#include "Utils/IntToString.h"
#include "Utils/StringList.h"
#include "Utils/ErrorsSubsystem.h"

#include "UlvTypes.h"
#include "UdpLogDBParams.h"


namespace Ulv
{
    // Parsed command line params
    class StartupParams
    {
        typedef StartupParams T;

        struct PortAndType
        {
            int Port;
            UdpPortDataType Type;

            PortAndType(int port, UdpPortDataType type) : Port(port), Type(type)
            {
                ESS_ASSERT(port >= 0);
            }
        };

        std::vector<PortAndType> m_ports;
        UdpLogDBParams m_dbParams;
        bool m_isValid;

        void SavePortAndType(const std::vector<std::string> &params); // can throw
        void SaveRecordCount(const std::string &param); // can throw
        void SaveDisableRecording();
        void SaveRecordingIntervalMs(const std::string &param); // can throw
        void SaveRecordingCount(const std::string &param); // can throw
        void CheckIfValid() const { ESS_ASSERT(IsValid()); }

    public:
        ESS_TYPEDEF(CommandLineError);
        ESS_TYPEDEF(HelpRequested);

    public:
        StartupParams(); // can throw

        void Parse(const Utils::StringList &args); // can throw CommandLineError or HelpRequested

        int Port(size_t index) const;
        UdpPortDataType PortDataType(size_t index) const;
        size_t PortsCount() const;

        const UdpLogDBParams& DBParams() const;

        bool IsValid() const { return m_isValid; }
    };
    
} // namespace Ulv


#endif
