
#include "stdafx.h"
#include "TelnetServerSession.h"


namespace Telnet
{
    // implement TelnetSession
    void TelnetServerSession::ParseCmd(byte cmd, byte opt)
    {
        byte outCmd = CaseReplyCommand(cmd, opt); 

        //CheckEstablish(outCmd, opt);
        
        SendToSocketCmd(outCmd, opt);

        // if option need to be initializated 
        // we send CMD_SB after CMD_DO
        if ((outCmd == CMD_DO) 
            && (m_optList.IsRequestInit(opt)))
        {
            SendToSocketSb(opt); 
        }

        // if we in GA mode, we should after last command send
        // CMD_GA which will indicate to other side, 
        // that it could start transmiting 
        if (m_optList.SendingAsHalfDuplexTerminal()) 
            SendToSocketCmd(CMD_GA);
        
    }

    // ---------------------------------------------------

    void TelnetServerSession::ParseSb(byte opt, const QByteArray &params)
    {
        if (!m_optList.IsExists(opt)) return;

        QByteArray tmp = TelnetHelpers::CutDoubleIacs(params);

        switch (opt)
        {
        case (OPT_NAWS): 
            ParseSbNaws(tmp);
            break;

        case (OPT_TTYPE):
            ParseSbTtype(tmp); 
            break;

        case (OPT_NEW_ENVIRON):         // equal
        case (OPT_OLD_ENVIRON):
            ParseSbEnviron(tmp); 
            break;

        case (OPT_XDISPLOC):
            ParseSbXdisploc(tmp);
            break;

        default:
            break;
        }

        // disable opt
        //m_optList.Disable(opt);
    }

    // ---------------------------------------------------

    void TelnetServerSession::ParseText(const QByteArray &data)
    {
        QByteArray res = TH::CutDoubleIacs(data);
        QString msg(TH::ToQString(res, getSendMode(),
                                  m_codecName));
        PutMsg(this, &T::OnTelnetDataInd, shared_ptr<QString>(new QString(msg)));
    }

    // ---------------------------------------------------

    void TelnetServerSession::SendToOwnerDataInd(const QString &data)
    {
        PutMsg(this, &T::OnTelnetDataInd, shared_ptr<QString>(new QString(data)));
    }

    // ---------------------------------------------------

    void TelnetServerSession::SendToOwnerDiscInd()
    {
        PutMsg(this, &T::OnTelnetDiscInd);
    }

    // ---------------------------------------------------

    void TelnetServerSession::SendToOwnerSocketErrorInd(shared_ptr<iNet::SocketError> error)
    {
        PutMsg(this, &T::OnTelnetSocketErrorInd, error);
    }

    // ---------------------------------------------------

    void TelnetServerSession::StartSession()
    {
        /*
        // start establish
        SendToSocketCmd(CMD_WILL, OPT_ECHO);
        m_optList.SetRequested(OPT_ECHO);
        */
        
        // start establish
        /*
        SendToSocketCmd(CMD_DO, OPT_TSPEED);
        m_optList.SetRequested(OPT_TSPEED);
        
        SendToSocketCmd(CMD_DO, OPT_TTYPE);
        m_optList.SetRequested(OPT_TTYPE);

        SendToSocketCmd(CMD_DO, OPT_XDISPLOC);
        m_optList.SetRequested(OPT_XDISPLOC);

        SendToSocketCmd(CMD_DO, OPT_NEW_ENVIRON);
        m_optList.SetRequested(OPT_NEW_ENVIRON);
        */

        SendToSocketCmd(CMD_WILL, OPT_SGA);
        m_optList.SetRequested(OPT_SGA);

        SendToSocketCmd(CMD_DO, OPT_ECHO);
        m_optList.SetRequested(OPT_ECHO);

        /*
        SendToSocketCmd(CMD_DO, OPT_NAWS);
        m_optList.SetRequested(OPT_NAWS);
        
        SendToSocketCmd(CMD_WILL, OPT_STATUS);
        m_optList.SetRequested(OPT_STATUS);

        SendToSocketCmd(CMD_DO, OPT_LFLOW);
        m_optList.SetRequested(OPT_LFLOW);
        */
        
        SendToSocketCmd(CMD_WILL, OPT_ECHO);
        m_optList.SetRequested(OPT_ECHO);
    }

    // ---------------------------------------------------

    TelnetDataLogger& TelnetServerSession::GetLogger()
    {
        return m_logger;
    }

    // ---------------------------------------------------
    // handlers
    byte TelnetServerSession::CaseReplyCommand(byte inCmd, byte inOpt)
    {
        ESS_ASSERT(!m_optList.IsEmpty());
        byte outCmd = CMD_NOP;

        switch (inCmd)
        {
        case (CMD_WILL): 
            if (!m_optList.IsExists(inOpt))
            {
                outCmd = CMD_DONT;
                break;
            }

            if (inOpt == OPT_ECHO) CheckEstablish(CMD_WILL);
            
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

                    outCmd = CMD_DO;
                }

                m_optList.SetRequested(inOpt, false);
            }
            break;

        case (CMD_WONT):
            m_optList.Disable(inOpt);
            CheckChangeMode(inOpt, false);

            if (inOpt == OPT_ECHO) CheckEstablish(CMD_WONT);
            break;

        case (CMD_DO):
            if ((m_optList.IsExists(inOpt))
                && (!m_optList.IsEnabled(inOpt)))
            {
                CheckChangeMode(inOpt, true);
                m_optList.Enable(inOpt);

                if (inOpt == OPT_ECHO)
                {
                    // outCmd = CMD_DO;
                    m_optList.Disable(inOpt);
                    break;
                }

                if (!m_optList.IsRequested(inOpt))
                {
                    outCmd = CMD_WILL;
                }

                m_optList.SetRequested(inOpt, false);
                
                break;
            }
            
            if (!m_optList.IsExists(inOpt))
            {
                outCmd = CMD_WONT;
            }

            break;

        case (CMD_DONT):
            // erase opt from list to make optionList 
            // of client and server cymmetrical
            if ((getState() != EstablishedSt)
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

    /*
    void TelnetServerSession::CheckEstablish(byte cmd, byte opt)
    {
        // don't reply to OPT_ECHO signals that we finish setup 
        // and establish connection
        if ((getState() != EstablishedSt) 
            && (cmd == CMD_NOP)
            && (opt == OPT_ECHO))
        {
            setState(EstablishedSt);
            PutMsg(this, &T::OnTelnetEstablishInd);
        }
    }
    */

    void TelnetServerSession::CheckEstablish(byte cmd)
    {
        // don't reply to OPT_ECHO signals that we finish setup 
        // and establish connection
        if (getState() == EstablishedSt) return; 

        ESS_ASSERT((cmd == CMD_WILL) || (cmd == CMD_WONT));

        if (cmd == CMD_WILL) m_connectedWithLinuxClient = false;
        else m_connectedWithLinuxClient = true;
        
        setState(EstablishedSt);
        PutMsg(this, &T::OnTelnetEstablishInd);
    }


    // ---------------------------------------------------

    void TelnetServerSession::CheckChangeMode(byte opt, bool enable)
    {
        if (!TH::IsModeChangingOpt(opt)) return;

        // if disable
        SendingMode mode = AsciiMode;

        if (enable)
        { 
            TH::DisablePrevMode(m_optList, opt);
            mode = TH::CaseModeByOpt(opt);
        }
        else if (opt == XOPT_TRANSMIT_CODEC) m_codecName.clear();

        PutMsg(this, &T::OnTelnetChangeSendingModeInd, mode);
    }

    // ---------------------------------------------------

    void TelnetServerSession::SendToSocketSb(byte opt)
    {
        QByteArray extended;

        if ((opt == OPT_NEW_ENVIRON)
            || (opt == OPT_OLD_ENVIRON))
        {
            if (m_varList.empty()) Append(extended, VOPT_VAR);
            if (m_userVarList.empty()) Append(extended, VOPT_USERVAR);

            for (size_t i = 0; i < m_varList.size(); ++i)
            {
                Append(extended, VOPT_VAR);
                extended += QByteArray(m_varList.at(i).c_str());
            }

            for (size_t i = 0; i < m_userVarList.size(); ++i)
            {
                Append(extended, VOPT_USERVAR);
                extended += QByteArray(m_varList.at(i).c_str());
            }
        }

        QByteArray res(TH::MakeServerSb(opt, extended));
        SendBytes(res);

        m_logger.Add(res, false, Telnet::TelnetDataLogger::SB); // output
    }

    // ---------------------------------------------------
    // SB parcer specializations
    void TelnetServerSession::ParseSbTtype(const QByteArray &params)
    {
        // IAC SB TTYPE IS val IAC SE
        // params ==  val
        QString msg("Terminal type is undefined.");
        
        if (params.isEmpty())  
        {
            PutMsg(this, &T::OnTelnetDataInd, shared_ptr<QString>(new QString(msg)));
            return;
        }

        msg = QString("Terminal type is ");
        msg += TH::ToQString(TH::CutDoubleIacs(params), 
                             getSendMode() /* m_sendMode */, 
                             m_codecName);
        PutMsg(this, &T::OnTelnetDataInd, shared_ptr<QString>(new QString(msg)));
    }

    // ---------------------------------------------------

    void TelnetServerSession::ParseSbNaws(const QByteArray &params)
    {
        // IAC SB NAWS width width height height IAC SE
        // params == width width height height
        // if some width/height byte was equal IAC it's doubled
        if (params.isEmpty()) return;

        int width = (int)TypeConvertion::ToWord(At(params, 0), At(params, 1));
        int height = (int)TypeConvertion::ToWord(At(params, 2), At(params, 3));
        PutMsg(this, &T::OnTelnetChangeWinSizeInd,
               shared_ptr<WinSize>(new WinSize(width, height)));
    }

    // ---------------------------------------------------

    void TelnetServerSession::ParseSbEnviron(const QByteArray &params)
    {
        // IAC SB NEW-ENVIRON IS VAR "USER" VALUE "ME" 
        // USERVAR "SHELL" VALUE "/bin/csh" IAC SE
        // params == AR "USER" VALUE "ME" USERVAR "SHELL" VALUE "/bin/csh"
       
        if (params.isEmpty()) 
        {
            QString msg("Environmental variables are undefined.");
            PutMsg(this, &T::OnTelnetDataInd, shared_ptr<QString>(new QString(msg)));
            return;
        }

        std::stringstream res("Environmental variables: ");

        // start from first VAR's name
        for (int i = 1; i < params.size(); ++i)
        {
            switch (params.at(i))
            {
            case (VOPT_VAR):
                res << "VAR = ";
                break;

            case (VOPT_USERVAR):
                res << "USERVAR = ";
                break;

            case (VOPT_VALUE):
                break;

            default:
                res << params.at(i);
                break;
            }
        }
        PutMsg(this, &T::OnTelnetDataInd, shared_ptr<QString>(new QString(res.str().c_str())));
    }

    // ---------------------------------------------------

    void TelnetServerSession::ParseSbXdisploc(const QByteArray &params)
    {
        // IAC SB XDISPLOC IS x.y:z IAC SE
        // params = x.y:z

        QString msg("X-display-location undefined.");

        if (params.isEmpty()) 
        {
            PutMsg(this, &T::OnTelnetDataInd, shared_ptr<QString>(new QString(msg)));
            return;
        }

        msg = TH::ToQString(TH::CutDoubleIacs(params), 
                            getSendMode() /* m_sendMode */, 
                            m_codecName);
        PutMsg(this, &T::OnTelnetDataInd, shared_ptr<QString>(new QString(msg)));
    }

    // ---------------------------------------------------
    // msg events to owner
    void TelnetServerSession::OnTelnetDataInd(shared_ptr<QString> pData)
    {
        m_pOwner->TelnetDataInd(*pData);
    }

    // ---------------------------------------------------

    void TelnetServerSession::OnTelnetDiscInd()
    {
        m_pOwner->TelnetDiscInd();
    }

    // ---------------------------------------------------

    void TelnetServerSession::OnTelnetEstablishInd()
    {
        m_pOwner->TelnetEstablishInd();
    }

    // ---------------------------------------------------

    void TelnetServerSession::OnTelnetChangeWinSizeInd(shared_ptr<WinSize> pSize)
    {
        m_pOwner->TelnetChangeWinSizeInd(pSize->getWidth(), pSize->getHeight());
    }

    // ---------------------------------------------------

    void TelnetServerSession::OnTelnetChangeSendingModeInd(SendingMode mode)
    {
        m_pOwner->TelnetChangeSendingModeInd(mode);
    }

    // ---------------------------------------------------

    void TelnetServerSession::OnTelnetSocketErrorInd(shared_ptr<iNet::SocketError> error)
    {
        m_pOwner->TelnetSocketErrorInd(error);
    }

    // ----------------------------------------------------

    void TelnetServerSession::OnSendCmd(boost::shared_ptr<CmdAndOpt> pData)
    {
        if (pData->getCmd() == CMD_SB) 
        {
            /*
            QByteArray res(TH::MakeServerSb(pData->getOpt()));
            SendBytes(res);

            m_logger.Add(res, false, Telnet::TelnetDataLogger::SB);
            */

            SendToSocketSb(pData->getOpt());
            return;
        }
        
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

    // ----------------------------------------------------

    void TelnetServerSession::OnSend(boost::shared_ptr<QString> pData)
    {
        QByteArray res = TH::ToQByteArray(*pData, 
                                          getSendMode(), 
                                          m_codecName);
        TelnetSession::SendBytes(TH::DoubleIacs(res));

        m_logger.AddStr(*pData, false);
    }

    // ----------------------------------------------------

    void TelnetServerSession::Send(const QString &data)
    {
        PutMsg(this, &T::OnSend, boost::shared_ptr<QString>(new QString(data)));
    }

    // ----------------------------------------------------

    void TelnetServerSession::SendCmd(byte cmd, byte opt)
    {
        PutMsg(this, &T::OnSendCmd, 
               shared_ptr<CmdAndOpt>(new CmdAndOpt(cmd, opt)));
    }

} // namespace Telnet
