#ifndef __TELNETCOMMANDS__
#define __TELNETCOMMANDS__

namespace Telnet
{

    enum Command
    {
        // ...
        CMD_SE = 240,
        CMD_NOP = 241,
        // urgent notifications
        CMD_DM = 242,
        CMD_BREAK = 243,
        CMD_IP = 244,
        CMD_AO = 245,
        CMD_AYT = 246,
        CMD_EC = 247,
        CMD_EL = 248,
        // --------------------
        CMD_GA = 249,
        CMD_SB = 250,
        CMD_WILL = 251,
        CMD_WONT = 252,
        CMD_DO = 253,
        CMD_DONT = 254,
        CMD_IAC = 255,
    };

    enum Option
    {   
        OPT_TRANSMIT_BINARY = 0, // +
        OPT_ECHO = 1,            // +
        OPT_SGA = 3,             // +
        //OPT_STATUS = 5,
        OPT_TTYPE = 24,          // +
        OPT_NAWS = 31,           // +
        OPT_TSPEED = 32,         // +
        //OPT_LFLOW = 33,
        OPT_XDISPLOC = 35,
        OPT_OLD_ENVIRON = 36,    // +
        OPT_NEW_ENVIRON = 39,    // +
        OPT_EXTOPL = 255,
    };

    enum SubOption
    {
        SOPT_IS = 0,
        SOPT_SEND = 1,
        SOPT_INFO = 2,
    };

    enum VarOption
    {
        VOPT_VAR = 0,
        VOPT_VALUE = 1,
        VOPT_ESC = 2,
        VOPT_USERVAR = 3,
    };

    enum ExtendedOption
    {
        XOPT_TRANSMIT_UNICODE = 2,
        XOPT_TRANSMIT_CODEC = 4,
    };

} // namespace Telnet

#endif
