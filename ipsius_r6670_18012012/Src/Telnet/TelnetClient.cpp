
#include "stdafx.h"
#include "TelnetClient.h"
#include "TelnetCommands.h"

namespace 
{
    enum
    {
        CDefaultWinWidth = 800,
        CDefaultWinHeidht = 600,
    };

    // -------------------------------

    using namespace std;
    using namespace Telnet;

    string GetEnvironVarValue(const string &name, bool isUserVar)
    {
        string val;

        // ...

        return val;
    }

    // ----------------------------------------------------

    //  <name, value>
    map<string, string> GetAllEnvironVarValues(bool isUserVar)
    {
        map<string, string> varMap;

        // ...

        return varMap;
    }

    // ----------------------------------------------------

    QByteArray ParseVarMap(bool userVar)
    {
        map<string, string> varMap = GetAllEnvironVarValues(userVar);

        if (varMap.empty()) return QByteArray();

        map<string, string>::iterator i;
        QByteArray res; 
        for (i = varMap.begin(); i != varMap.end(); ++i)
        {
            res += TelnetHelpers::MakeClientSbEnvironParams(i->first,
                                                           i->second,
                                                           userVar);
        }

        return res;
    }

} // namespace

// ----------------------------------

namespace Telnet
{
    TelnetClient::TelnetInitParams::TelnetInitParams()
    : m_winWidth(CDefaultWinWidth), 
      m_winHeight(CDefaultWinHeidht)
    {}

    // ---------------------------------------------------

    // implement TelnetSession
    void TelnetClient::ParseText(const QByteArray &data)
    {
        QByteArray res = TH::CutDoubleIacs(data);
        QString msg(TH::ToQString(res, 
                                  getSendMode(),
                                  m_params.getCodecName()));
        PutMsg(this, &T::OnTelnetDataInd, shared_ptr<QString>(new QString(msg)));
    }

    // ---------------------------------------------------

    void TelnetClient::ParseCmd(byte cmd, byte opt)
    {
        byte outCmd = CaseReplyCommand(cmd, opt); 

        //CheckEstablish(outCmd, opt);

        if (outCmd != CMD_NOP) 
            SendToSocketCmd(outCmd,opt);
    }

    // ---------------------------------------------------

    void TelnetClient::ParseSb(byte opt, const QByteArray &params)
    {
        if (!m_optList.IsExists(opt)) return;

        switch (opt)
        {
        case (OPT_NAWS): 

            SendToSocketSbNaws();
            break;

        case (OPT_TTYPE):
            SendToSocketSbTtype(); 
            break;

        case (OPT_NEW_ENVIRON):  
            SendToSocketSbEnviron(OPT_NEW_ENVIRON, params); 
            break;

        case (OPT_OLD_ENVIRON):
            SendToSocketSbEnviron(OPT_OLD_ENVIRON, params); 
            break;

        case (OPT_XDISPLOC):
            SendToSocketSbXdisploc();
            break;

        default:
            break;
        }
    }

    // ---------------------------------------------------

    /*
    void TelnetClient::CheckEstablish(byte cmd, byte opt)
    {
        // no-reply to OPT_ECHO signals that we finish setup 
        // and establish connection
        if ((getState() != TelnetSession::EstablishedSt) 
            && (cmd == CMD_WONT)
            && (opt == OPT_ECHO))
        {
            setState(TelnetSession::EstablishedSt);
            PutMsg(this, &T::OnTelnetEstablishInd);
        }
    }
    */

    void TelnetClient::CheckEstablish()
    {
        // no-reply to OPT_ECHO signals that we finish setup 
        // and establish connection
        if (getState() == TelnetSession::EstablishedSt) return;
           
        setState(TelnetSession::EstablishedSt);
        PutMsg(this, &T::OnTelnetEstablishInd);
    }

    // ---------------------------------------------------

    void TelnetClient::StartSession()
    {
        SendToSocketCmd(CMD_DO, OPT_SGA);
        m_optList.SetRequested(OPT_SGA);
        m_optList.Enable(OPT_SGA);

        // OnSendWinSize(shared_ptr<WinSize>(new WinSize()));
        
        // OnSendTerminalType(shared_ptr<QString>(new QString(m_params.getTtype())));
    }

    // ---------------------------------------------------

    void TelnetClient::SendToOwnerDataInd(const QString &data)
    {
        PutMsg(this, &T::OnTelnetDataInd, shared_ptr<QString>(new QString(data)));
    }

    // ---------------------------------------------------

    void TelnetClient::SendToOwnerDiscInd()
    {
        PutMsg(this, &T::OnTelnetDiscInd);
    }

    // ---------------------------------------------------

    void TelnetClient::SendToOwnerSocketErrorInd(shared_ptr<iNet::SocketError> error)
    {
        PutMsg(this, &T::OnTelnetSocketErrorInd, error);
    }

    // ---------------------------------------------------

    TelnetDataLogger& TelnetClient::GetLogger()
    {
        return m_logger;
    }

    // ---------------------------------------------------
    // handlers
    byte TelnetClient::CaseReplyCommand(byte inCmd, byte inOpt)
    {
        ESS_ASSERT(!m_optList.IsEmpty());
        byte outCmd = CMD_NOP;

        switch (inCmd)
        {
        case (CMD_WILL): 
            //if (inOpt != OPT_SGA) outCmd = CMD_DONT;

            if (!m_optList.IsExists(inOpt))
            {
                outCmd = CMD_DONT;
                break;
            }

            if (!m_optList.IsEnabled(inOpt))
            {
                if (inOpt != OPT_ECHO) m_optList.Enable(inOpt);
                CheckChangeMode(inOpt, true);

                if ((!m_optList.IsRequested(inOpt))
                    ||(inOpt == OPT_ECHO))
                {
                    outCmd = CMD_DO;
                }

                m_optList.SetRequested(inOpt, false);
            }
            break;

        case (CMD_WONT):
            m_optList.Disable(inOpt);
            CheckChangeMode(inOpt, false);
            break;

        case (CMD_DO):
            
            if (!m_optList.IsExists(inOpt))
            {
                outCmd = CMD_WONT;
                break;
            }

            if (inOpt == OPT_ECHO)
            {
                outCmd = CMD_WONT;
                CheckEstablish();
                break;
            }

            if ((!m_optList.IsEnabled(inOpt))
                && ((!m_optList.IsRequestInit(inOpt))
                    ||(TH::IsModeChangingOpt(inOpt))))
            {
                CheckChangeMode(inOpt, true);
                
                if (!m_optList.IsRequested(inOpt))
                {
                    outCmd = CMD_WILL;
                }

                m_optList.Enable(inOpt);
                m_optList.SetRequested(inOpt, false);
            }
            
            /*
            if (!m_optList.IsEnabled(inOpt)) 
            {
                if (!m_optList.IsRequested(inOpt))
                {
                    if ((!m_optList.IsRequestInit(inOpt))
                        || (TH::IsModeChangingOpt(inOpt)))
                    {
                        CheckChangeMode(inOpt, true);
                        m_optList.Enable(inOpt);
                    }

                    outCmd = CMD_WILL;
                }

                m_optList.SetRequested(inOpt, false);
            }
            */
            break;

        case (CMD_DONT):
            // erase opt from list to make optionList 
            // of client and server cymmetrical
            if ((getState() != TelnetSession::EstablishedSt)
                && (!m_optList.IsEnabled(inOpt)))
            {
                m_optList.Remove(inOpt);
                break;
            }

            m_optList.Disable(inOpt);
            CheckChangeMode(inOpt, false);
            break;

        default:
            break;
        }

        return outCmd;
    }

    // ---------------------------------------------------

    void TelnetClient::CheckChangeMode(byte opt, bool enable)
    {
        if (!TH::IsModeChangingOpt(opt)) return;

        // if disable
        SendingMode mode = AsciiMode;

        if (enable)
        {
            TelnetHelpers::DisablePrevMode(m_optList, opt);
            mode = TH::CaseModeByOpt(opt);
        }
        else if (opt == XOPT_TRANSMIT_CODEC) m_params.setCodecName(); // clear

        PutMsg(this, &T::OnTelnetChangeModeInd, mode);
    }

    // ---------------------------------------------------

    SendingMode TelnetClient::getSendMode()
    {
        return m_optList.getOwnerSendingMode();
    }

    // ---------------------------------------------------

    void TelnetClient::SendToSocketSb(const QByteArray &data)
    {
        SendBytes(data);
        m_logger.Add(data, false, Telnet::TelnetDataLogger::SB); // output
    }

    // ---------------------------------------------------
    // data == params from ParseSb()
    void TelnetClient::SendToSocketSbEnviron(byte environOpt, 
                                             const QByteArray &data)
    {
        if (data.size() == 1)
        {
            bool isUserVar = (At(data, 0) == VOPT_USERVAR) ? true : false;
            QByteArray params = ParseVarMap(isUserVar);
            SendBytes(TH::MakeClientSb(environOpt, params));
            return;
        }

        std::stringstream varName;
        QByteArray params;
        
        // parse data
        int i = 0;
        while(i < data.size())
        {
            switch (At(data, i))
            {
            case (VOPT_VAR):
                if (At(data, ++i) == VOPT_USERVAR)
                {
                    params += ParseVarMap(true);
                    break;
                }

                if (varName == 0) break;
                params += TH::MakeClientSbEnvironParams(varName.str(), 
                                                        GetEnvironVarValue(varName.str(),
                                                                           false),
                                                        false);
                varName.clear();
                break;

            case (VOPT_USERVAR):
                if (varName == 0) break;
                params += TH::MakeClientSbEnvironParams(varName.str(), 
                                                        GetEnvironVarValue(varName.str(),
                                                                           true), 
                                                        true);
                varName.clear();
                break;

            default:
                varName << (char)data.at(i);
                break;
            }

            ++i;
        }

        SendToSocketSb(TH::MakeClientSb(environOpt, params));
    }
    
    // ---------------------------------------------------

    void TelnetClient::SendToSocketSbTtype()
    {
        SendToSocketSb(TH::MakeClientSb(OPT_TTYPE, 
                       TypeConvertion::FromAscii(m_params.getTtype())));
    }

    // ---------------------------------------------------

    void TelnetClient::SendToSocketSbNaws()
    {
        QByteArray params = TypeConvertion::ToQByteArray(m_params.getWinWidth())  
                            + TypeConvertion::ToQByteArray(m_params.getWinHeight());
        
        SendToSocketSb(TH::MakeClientSb(OPT_NAWS, params));
    }

    // ---------------------------------------------------

    void TelnetClient::SendToSocketSbXdisploc()
    {
        //<host>:<dispnum>[.<screennum>]
        QByteArray loc(TypeConvertion::FromAscii(m_params.getlocation().AsQString(),
                                                 m_params.getCodecName()));

        SendToSocketSb(TH::MakeClientSb(OPT_XDISPLOC, loc));
    }

    // ---------------------------------------------------

    void TelnetClient::SendToSocketSbXopt(SendingMode mode)
    {
        SendToSocketSb(TH::MakeXoptCmd(CMD_WILL, TH::CaseOptionByMode(mode)));
    }

    // ---------------------------------------------------
    // messages
    void TelnetClient::ChangeSendingModeReq(SendingMode mode, const string &codecName)
    {
        if (!codecName.empty()) m_params.setCodecName(codecName);

        PutMsg(this, &T::OnChangeSendingModeReq, mode);
    }

    // ---------------------------------------------------

    void TelnetClient::SendWinSize(word width, word height)
    {
        PutMsg(this, &T::OnSendWinSize,
               shared_ptr<WinSize>(new WinSize(width, height)));
    }

    // ---------------------------------------------------

    void TelnetClient::SendTerminalType(const QString &name)
    {
        PutMsg(this, &T::OnSendTerminalType, 
               shared_ptr<QString>(new QString(name)));
    }

    // ---------------------------------------------------

    void TelnetClient::SendXDisplayLocation(const Location &loc)
    {
        PutMsg(this, &T::OnSendXDisplayLocation, 
               shared_ptr<Location>(new Location(loc)));
    }

    // ---------------------------------------------------

    void TelnetClient::Send(const QString &data)
    {
        PutMsg(this, &T::OnSend, shared_ptr<QString>(new QString(data)));
    }

    // ---------------------------------------------------

    void TelnetClient::SendCmd(byte cmd, byte opt)
    {
        PutMsg(this, &T::OnSendCmd, 
               boost::shared_ptr<CmdAndOpt>(new CmdAndOpt(cmd, opt)));
    }

    // ---------------------------------------------------
    // msg events from owner
    void TelnetClient::OnChangeSendingModeReq(SendingMode mode)
    {
        if (mode == BinaryMode)
        {
            m_optList.SetRequested(OPT_TRANSMIT_BINARY);
            //m_optList.Enable(OPT_TRANSMIT_BINARY);

            SendToSocketCmd(CMD_WILL, OPT_TRANSMIT_BINARY);
            return;
        }

        // if unicode or codec
        m_optList.SetRequested(OPT_EXTOPL);
        //m_optList.Enable(OPT_EXTOPL);

        SendToSocketCmd(CMD_WILL, OPT_EXTOPL);
        
        SendToSocketSbXopt(mode);
    }

    // ---------------------------------------------------

    void TelnetClient::OnSendWinSize(shared_ptr<WinSize> pSize)
    {
        if (!m_optList.IsExists(OPT_NAWS)) 
        {
            QString msg("Server doesn't support Window Size option.");
            PutMsg(this, &T::OnTelnetDataInd, 
                   shared_ptr<QString>(new QString(msg)));
            return;
        }

        m_params.setWinSize(pSize->getWidth(), pSize->getHeight());

        m_optList.SetRequested(OPT_NAWS);
        m_optList.Enable(OPT_NAWS);

        SendToSocketCmd(CMD_WILL, OPT_NAWS);
    }

    // ---------------------------------------------------

    void TelnetClient::OnSendTerminalType(shared_ptr<QString> pName)
    {
        if (!m_optList.IsExists(OPT_TTYPE)) 
        {
            QString msg("Server doesn't support Terminal Type option.");
            PutMsg(this, &T::OnTelnetDataInd, 
                   shared_ptr<QString>(new QString(msg)));
            return;
        }

        m_params.setTtype(*pName);
        
        m_optList.SetRequested(OPT_TTYPE);
        m_optList.Enable(OPT_TTYPE);

        SendToSocketCmd(CMD_WILL, OPT_TTYPE);
    }

    // ---------------------------------------------------

    void TelnetClient::OnSendXDisplayLocation(shared_ptr<Location> pLoc)
    {
        if (!m_optList.IsExists(OPT_XDISPLOC)) 
        {
            QString msg("Server doesn't support X Display Location option.");
            PutMsg(this, &T::OnTelnetDataInd, 
                   shared_ptr<QString>(new QString(msg)));
            return;
        }

        m_params.setLocation(*pLoc);
        
        m_optList.SetRequested(OPT_XDISPLOC);
        m_optList.Enable(OPT_XDISPLOC);

        SendToSocketCmd(CMD_WILL, OPT_XDISPLOC);
    }

    // ---------------------------------------------------
    // msg events to owner
    void TelnetClient::OnTelnetDataInd(shared_ptr<QString> pData)
    {
        m_pOwner->TelnetDataInd(*pData);
    }

    // ---------------------------------------------------

    void TelnetClient::OnTelnetDiscInd()
    {
        m_pOwner->TelnetDiscInd();
    }

    // ---------------------------------------------------

    void TelnetClient::OnTelnetEstablishInd()
    {
        m_pOwner->TelnetEstablishInd();
    }

    // ---------------------------------------------------

    void TelnetClient::OnTelnetChangeModeInd(SendingMode mode)
    {
        m_pOwner->TelnetChangeModeInd(mode);
    }

    // ---------------------------------------------------

    void TelnetClient::OnTelnetSocketErrorInd(shared_ptr<iNet::SocketError> error)
    {
        m_pOwner->TelnetSocketErrorInd(error);
    }

    // ---------------------------------------------------

    void TelnetClient::OnSendCmd(boost::shared_ptr<CmdAndOpt> pData)
    {
        /*
        if (data->getCmd() == CMD_SB) 
        {
            QByteArray res(TH::MakeClientSb(data->getOpt()));
            SendBytes(res);

            m_logger.Add(res, false, TelnetDataLogger::SB);

            return;
        }
        */
        
        if ((pData->getCmd() == CMD_WILL) 
            || (pData->getCmd() == CMD_DO))
        {
            if (TH::IsModeChangingOpt(pData->getOpt()))
            {
                m_optList.Enable(pData->getOpt());
                CheckChangeMode(pData->getOpt(), true);
            }
        }

        if ((pData->getCmd() == CMD_WONT) 
            || (pData->getCmd() == CMD_DONT))
        {
            m_optList.Disable(pData->getOpt());
            CheckChangeMode(pData->getOpt(), false);
        }

        m_optList.SetRequested(pData->getOpt());

        SendToSocketCmd(pData->getCmd(), pData->getOpt());
    }

    // ---------------------------------------------------

    void TelnetClient::OnSend(boost::shared_ptr<QString> pData)
    {
        QByteArray res = TH::DoubleIacs(TH::ToQByteArray(*pData, 
                                          getSendMode(), 
                                          m_params.getCodecName()));
        TelnetSession::SendBytes(res);

        m_logger.AddStr(*pData, false); // output
    }
    
} // namespace Telnet
