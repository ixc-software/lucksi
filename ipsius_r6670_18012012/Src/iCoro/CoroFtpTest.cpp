#include "stdafx.h"

#include "Utils/ManagedList.h"
#include "Utils/UtilsClassStr.h"
#include "Utils/RawCRC32.h"
#include "Utils/IntToString.h"

#include "iNet/MsgTcpServer.h"
#include "iNet/ITcpServerToUser.h"
#include "iNet/ITcpSocketToUser.h"
#include "iNet/ITcpSocket.h"
#include "iNet/SocketData.h"
#include "iNet/MsgTcpSocket.h"

#include "CoroMsg.h"

#include "CoroFtpTest.h"

// -----------------------------------------------------------

namespace
{   
    using boost::shared_ptr;
    using Platform::dword;
    using Platform::byte;

    typedef boost::function<void (QString)> CompletedFn;

    const QByteArray CPassword = "1234";
    const QByteArray CRespOk   = "OK";

    enum 
    {
        CCmdSplitChar = 13,
    };

    ESS_TYPEDEF(BadCommand);

    class Command
    {
        QByteArray m_name;
        QList<QByteArray> m_args;

        static Command Join(const char *pCmd, QByteArray arg0 = "", QByteArray arg1 = "")
        {
            ESS_ASSERT(pCmd != 0);

            QByteArray res;

            res += pCmd;
            if ( arg0.size() ) res += " " + arg0;
            if ( arg1.size() ) res += " " + arg1;

            return Command(res);
        }

        static QByteArray ListJoin(const QList<QByteArray> &args, const QByteArray &sep)
        {
            QByteArray res;

            for (int i = 0; i < args.size(); ++i) 
            {
                if (i) res += sep;
                res += args.at(i);
            }

            return res;             
        }

        static QByteArray BinDataToHex(const QByteArray &data)
        {
            QByteArray res;
            res.reserve(data.size() * 2);

            for(int i = 0; i < data.size(); ++i)
            {
                std::string v = Utils::IntToHexString( (byte)data.at(i), false, 1);
                ESS_ASSERT( v.length() == 2);
                res.append( v.c_str() );
            }

            return res;
        }

        static QByteArray DwToHex(dword val)
        {
            return Utils::IntToHexString(val, false).c_str();
        }

    public:

        Command(QByteArray &data)
        {
            QList<QByteArray> lx = data.split(' ');
            if ( lx.empty() ) ESS_THROW_MSG(BadCommand, data.data());

            for(int i = 0; i < lx.size(); ++i)
            {
                const QByteArray &c = lx.at(i);
                if ( c.isEmpty() ) ESS_THROW_MSG(BadCommand, data.data());
                if (i == 0) m_name = c;
                      else  m_args.push_back(c);
            }
        }

        const QByteArray& Name() const                  { return m_name; }
        const int ArgCount() const                      { return m_args.size(); }
        const QByteArray& Arg(int index) const          { return m_args.at(index); }
        const QByteArray& operator[](int index) const   { return Arg(index); }

        const QString ToString() const
        {
            return QString( AsPacket() );
        }

        QByteArray AsPacket() const 
        {
            QByteArray res = m_name;

            if ( m_args.size() )
            {
                res += " " + ListJoin(m_args, " ");
            }

            res.append(CCmdSplitChar);

            return res;
        }

    // commands
    public:

        // commands
        UTILS_CLASS_STR(CLogin,         "login");
        UTILS_CLASS_STR(CFileCreate,    "file_create");
        UTILS_CLASS_STR(CFileWrite,     "file_write");
        UTILS_CLASS_STR(CFileClose,     "file_close");
        UTILS_CLASS_STR(CLogout,        "logout");

        // responses
        UTILS_CLASS_STR(CSrvResponse, "srv_response");

        static Command MakeLogin(QByteArray login, QByteArray password) { return Join(CLogin(),         login, password); }
        static Command MakeFileCreate(QByteArray fileName)              { return Join(CFileCreate(),    fileName); }
        static Command MakeFileWrite(QByteArray binData)                { return Join(CFileWrite(),     BinDataToHex(binData)); }
        static Command MakeFileClose(Platform::dword crc)               { return Join(CFileClose(),     DwToHex(crc)); }
        static Command MakeLogout()                                     { return Join(CLogout()         ); }

        static Command MakeSrvResponse(QByteArray msg)                  { return Join(CSrvResponse(),   msg); }

    };

    // --------------------------------------------------

    class CmdReciever : boost::noncopyable
    {
        QByteArray m_buff;

    public:
        CmdReciever() {}

        void PutData(const QByteArray &data)
        {
            m_buff.append(data);
        }

        bool CommandReady() const
        {
            return (m_buff.indexOf(CCmdSplitChar) >= 0);
        }

        Command GetCommand()
        {
            int pos = m_buff.indexOf(CCmdSplitChar);
            ESS_ASSERT(pos >= 0);

            Command c( m_buff.mid(0, pos) );
            m_buff = m_buff.mid(pos + 1);
            return c;
        }
    };

    // --------------------------------------------------

    class MsgCommand : public iCoro::CoroMsg::IMsg
    {
        Command m_cmd;
    public:
        MsgCommand(const Command &c) : m_cmd(c) {}
        const Command& Cmd() const { return m_cmd; }
    };

    class MsgDisconnected : public iCoro::CoroMsg::IMsg
    {
        shared_ptr<iNet::SocketError> m_err;
    public:
        MsgDisconnected(shared_ptr<iNet::SocketError> err) : m_err(err) 
        {
            ESS_ASSERT(err != 0);
        }
        const iNet::SocketError& Error() const { return *m_err; }
    };


}  // namespace

// -----------------------------------------------------------

namespace
{
    class Client;

    typedef boost::function<void (Client*, QString)> ClientCompletedFn;

    struct ClientSettings
    {
        QString SrcDir;
        
        ClientSettings(QString srcDir) : SrcDir(srcDir) { }
    };

    // ---------------------------------------------------------

    class Client : 
        public iNet::ITcpSocketToUser,
        public virtual Utils::SafeRefServer,
        iCoro::CoroMsg
    {
        ESS_TYPEDEF(RuntimeError);

        QStringList m_files;
        ClientCompletedFn m_callback;

        iNet::MsgTcpSocket m_sock;
        CmdReciever m_recv;

        void SendCmd(const Command &c)
        {
            m_sock.SendData( c.AsPacket() );
        }

        Command RecvCommmand()
        {
            shared_ptr<MsgCommand> cmd = GetMsg<MsgCommand>();
            return cmd->Cmd();
        }

        void RecvOk()
        {
            Command c = RecvCommmand();
            if ( c.Name() == Command::CSrvResponse() )
            {
                if ( (c.ArgCount() == 1) && (c[0] == CRespOk) )
                {
                    return;  // OK
                }
            }

            ESS_THROW_MSG( RuntimeError, "Unexpected response! " + c.ToString().toStdString() ); 
        }

        void ProcessFile(QString fileName)  // fiber context
        {
            const int CBlockSize = 1024;

            // open file 
            QFile f( fileName );
            if ( !f.open(QIODevice::ReadOnly) ) 
            {
                ESS_THROW_MSG( RuntimeError, "Can't open file " + fileName.toStdString() );
            }

            // 'create file'
            {
                QString shortName = QFileInfo(fileName).fileName();
                SendCmd( Command::MakeFileCreate( shortName.toAscii() ) );  
                RecvOk();
            }

            // process data
            dword crc = Utils::RawCRC32::InitialValue;

            while(true)
            {
                QByteArray data = f.read(CBlockSize);

                if ( data.size() )
                {
                    SendCmd( Command::MakeFileWrite(data) );
                    RecvOk();

                    crc = Utils::UpdateCRC32(data.data(), data.size(), crc, false);
                }

                if (data.size() != CBlockSize) break;  // eof
            }

            // 'file close'
            crc = Utils::UpdateCRC32(&crc /* dummi */, 0, crc, true);  // finalize
            SendCmd( Command::MakeFileClose(crc) );
            RecvOk();
        }

    // CoroMsg overrides 
    private:

        void CoroBody()  // override; fiber context
        {
            SetDefaultTimeout(5 * 1000);
            MsgHookThrow<MsgWaitTimeout> timeoutHook(*this);

            // login
            SendCmd( Command::MakeLogin("user", CPassword) );
            RecvOk();

            // files loop
            for(int i = 0; i < m_files.size(); ++i)
            {
                ProcessFile( m_files.at(i) );
            }

            // logout
            SendCmd( Command::MakeLogout() );
            RecvOk();
        }

        void CoroCompleted(const CoroCompletedInfo &info) // override
        {
            QString msg = info.OK() ? "" : info.ToString().c_str();
            m_callback(this, msg);
        }

    // ITcpSocketToUser impl
    private:

        void Connected(iNet::SocketId id)
        {
            CoroRun();
        }

        void ReceiveData(shared_ptr<iNet::SocketData> data)
        {
            m_recv.PutData( data->getData() );
            if ( m_recv.CommandReady() )
            {
                Command c = m_recv.GetCommand();
                PutMsg( new MsgCommand(c) );
            }            
        }

        void Disconnected(shared_ptr<iNet::SocketError> error)
        {
            PutMsg(new MsgDisconnected(error) );
        }


    public:

        Client(iCore::MsgThread &thread, const QStringList &files, 
            Utils::HostInf &remoteAddr, ClientCompletedFn callback) :
          iCoro::CoroMsg(thread),
          m_files(files),
          m_callback(callback),
          m_sock(thread, this)
        {
            m_sock.ConnectToHost(remoteAddr);
            // m_sock.LinkUserToSocket(this);     // ?!
        }
    };

    // ---------------------------------------------------------

    class ClientPool : public iCore::MsgObject
    {
        typedef ClientPool T;

        CompletedFn m_callback;
        Utils::ManagedList<Client> m_clients;

        void DeleteClient(Client *pC, QString info)
        {
            if ( info.size() )   // error
            {
                m_callback(info);
            }

            int i = m_clients.Find(pC);
            ESS_ASSERT(i >= 0);
            m_clients.Delete(i);

            // all finished
            if ( m_clients.IsEmpty() )
            {
                m_callback("");
            }
        }

        void OnClientCompleted(Client *pC, QString info)
        {
            PutFunctor( boost::bind(&T::DeleteClient, this, pC, info) );
        }

        static QStringList FilterFileList(QStringList files)
        {
            QStringList res;

            for(int i = 0; i < files.size(); ++i)
            {
                if ( files[i].contains(" ") ) continue;
                res.append( files[i] );
            }

            return res;
        }


    public:

        ClientPool(iCore::MsgThread &thread, ClientSettings &settings, 
            Utils::HostInf &remoteAddr, CompletedFn callback) : 
            iCore::MsgObject(thread),
            m_callback(callback)
        {
            QDir d(settings.SrcDir);
            if ( !d.exists() ) ESS_THROW_MSG(ESS::Exception, "Src dir not found!");

            QStringList files = d.entryList(QDir::Files, QDir::Name);
            if ( files.empty() ) ESS_THROW_MSG(ESS::Exception, "Src dir is empty!");

            files = FilterFileList(files);

            while( files.size() )
            {
                int filesToCopy = (m_clients.Size() & 0x01) ? 2 : 1;

                QStringList lx;
                while(filesToCopy-- > 0 && files.size())
                {
                    lx.push_back( d.absoluteFilePath( files.at(0) ) );
                    files.pop_front();
                }

                m_clients.Add( 
                    new Client(thread, lx, remoteAddr, boost::bind(&T::OnClientCompleted, this, _1, _2) ) 
                    );
            }
        }
    };

}  // namespace


// -----------------------------------------------------------

namespace
{
    class ServerSession;

    typedef boost::function<void (ServerSession*, QString)> ServerSessionCompletedFn;

    struct SrvSettings
    {
        QString DstDir;

        SrvSettings(QString dstDir) : DstDir(dstDir) { }
    };

    // -------------------------------------------------

    class ServerSession : 
        iCoro::CoroMsg, 
        public iNet::ITcpSocketToUser,
        public virtual Utils::SafeRefServer
    {
        ESS_TYPEDEF(RuntimeError);
        
        SrvSettings m_settings;
        shared_ptr<iNet::ITcpSocket> m_socket;
        ServerSessionCompletedFn m_callback;

        CmdReciever m_recv;

        void SendResponse(QString msg)
        {
            m_socket->SendData( Command::MakeSrvResponse( msg.toAscii() ).AsPacket() );
        }

        void SendOk()
        {
            SendResponse(CRespOk);
        }

        void SendErrorAndThrow(QString msg)
        {
            SendResponse(msg);
            ESS_THROW_MSG(RuntimeError, msg.toStdString());
        }

        Command RecvCommmand()
        {
            shared_ptr<MsgCommand> cmd = GetMsg<MsgCommand>();
            return cmd->Cmd();
        }

        void VerifyCmd(const Command &cmd, QByteArray cmdName, int argCount)
        {
            if ( cmdName != cmd.Name() ) 
            {
                SendErrorAndThrow("Unexpected cmd name");
            }

            if ( argCount != cmd.ArgCount() )
            {
                SendErrorAndThrow("Bad command arg count!" + cmd.ToString());
            }

            // OK, exit
        }

        void VerifyArgCount(const Command &cmd, int argCount)
        {
            if ( argCount != cmd.ArgCount() )
            {
                SendErrorAndThrow("Bad command arg count! " + cmd.ToString());
            }

            // OK, exit
        }

        dword HexToDw(const QByteArray &s)
        {
            dword res;
            if ( !Utils::HexStringToInt(s.data(), res) )
            {
                SendErrorAndThrow(QString("HexToDw ") + s);
            }

            return res;
        }

        QByteArray HexToBinData(const QByteArray &s)
        {
            int bytesCount = s.size() / 2;

            if (s.isEmpty() || (bytesCount * 2 != s.size())) 
            {
                SendErrorAndThrow( "Bad bin data size " + QString::number(s.size()) );
            }

            QByteArray res;
            res.reserve(bytesCount);  

            for(int i = 0; i < bytesCount; ++i)
            {
                Platform::byte c = HexToDw( s.mid(i * 2, 2) );
                res.append(c);
            }

            return res;
        }


        void ProcessFile(QFile &f) // fiber context
        {
            dword crc = Utils::RawCRC32::InitialValue;

            while(true)
            {
                Command cmd = RecvCommmand();

                // close file
                if ( cmd.Name() == Command::CFileClose() )
                {
                    VerifyArgCount(cmd, 1);

                    crc = Utils::UpdateCRC32(&crc /* dummi */, 0, crc, true); // finalize CRC
                    dword recvCrc = HexToDw(cmd[0]);                                           
                    if (crc != recvCrc) SendErrorAndThrow("Bad CRC");

                    SendOk();
                    break;   // exit
                }

                // data poriton
                VerifyCmd(cmd, Command::CFileWrite(), 1);

                QByteArray data = HexToBinData(cmd[0]);
                if ( f.write(data) != data.size() ) 
                {
                    SendErrorAndThrow("Write error!");
                }

                crc = Utils::UpdateCRC32(data.data(), data.size(), crc, false);
                SendOk();
            }
        }

        void Body()  // fiber context
        {
            // login
            {
                Command cmd = RecvCommmand();

                VerifyCmd(cmd, Command::CLogin(), 2);
                if (cmd[1] != CPassword) SendErrorAndThrow("Bad password");

                SendOk();
            }

            // process files
            while(true)
            {
                Command cmd = RecvCommmand();

                if ( cmd.Name() == Command::CLogout() ) 
                {
                    VerifyArgCount(cmd, 0);
                    SendOk();
                    break;  // session ended
                }

                // start new file
                {
                    VerifyCmd(cmd, Command::CFileCreate(), 1);

                    QString fileName = QDir(m_settings.DstDir).absoluteFilePath( cmd[0] );
                    if ( QFile(fileName).exists() ) SendErrorAndThrow("File exists");

                    QFile f(fileName);
                    if ( !f.open(QIODevice::WriteOnly) ) SendErrorAndThrow("Can't create file!");

                    SendOk();

                    ProcessFile(f);
                }                
            }

            // logout: nothing, just exit 
        }

    // CoroMsg overrides 
    private:

        void CoroBody()  // override; fiber context
        {
            SetDefaultTimeout(5 * 1000);
            MsgHookThrow<MsgWaitTimeout> timeoutHook(*this);

            try
            {
                Body();
                Wait(1000);
                m_socket->DisconnectFromHost();
            }
            catch(const std::exception &e)
            {                
                throw;  // just re-throw, exit with error 
            }
        }

        void CoroCompleted(const CoroCompletedInfo &info) // override
        {
            QString msg = info.OK() ? "" : info.ToString().c_str();
            m_callback(this, msg);
        }


    // iNet::ITcpSocketToUser impl
    private:

        void Connected(iNet::SocketId id)
        {
            ESS_HALT("imposible");
        }

        void ReceiveData(shared_ptr<iNet::SocketData> data)
        {
            m_recv.PutData( data->getData() );
            if ( m_recv.CommandReady() )
            {
                Command c = m_recv.GetCommand();
                PutMsg( new MsgCommand(c) );
            }
        }

        void Disconnected(shared_ptr<iNet::SocketError> error)
        {
            PutMsg(new MsgDisconnected(error) );
        }

    public:

        ServerSession(iCore::MsgThread &thread, const SrvSettings &settings, 
            shared_ptr<iNet::ITcpSocket> socket, 
            ServerSessionCompletedFn &callback) :
            iCoro::CoroMsg(thread),
            m_settings(settings),
            m_socket(socket),
            m_callback(callback)
        {
            m_socket->LinkUserToSocket(this);
            CoroRun();
        }

        ~ServerSession()
        {
            // clear all messages, SafeRef for socket can be stored here 
            Abort();   
        }
    };

    // -------------------------------------------------

    class Server : 
        public iCore::MsgObject, 
        public iNet::ITcpServerToUser,
        boost::noncopyable
    {
        SrvSettings m_settings;
        CompletedFn m_onSrvError;

        iNet::MsgTcpServer m_tcpServer;
        Utils::ManagedList<ServerSession> m_sessions;


        void CloseSession(ServerSession *pSender, QString info)
        {
            if ( !info.isEmpty() )
            {
                m_onSrvError(info);
            }

            int i = m_sessions.Find(pSender); 
            ESS_ASSERT( i >= 0 );
            m_sessions.Delete(i);
        }

        void OnSessionCompleted(ServerSession *pSender, QString info)
        {
            PutFunctor( boost::bind(&Server::CloseSession, this, pSender, info) );
        }

        void ClearDstDir()
        {
            QDir d(m_settings.DstDir);
            if ( !d.exists() ) ESS_THROW_MSG(ESS::Exception, "Dst dir not found!");

            QStringList files = d.entryList(QDir::Files, QDir::Name);
            for(int i = 0; i < files.size(); ++i)
            {
                QString fileName = files.at(i);
                if ( !d.remove(fileName) )
                {
                    ESS_THROW_MSG(ESS::Exception, "Can't delete file " + fileName.toStdString());
                }
            }
        }
       
    // ITcpServerToUser impl
    private:

        void NewConnection(iNet::SocketId id, shared_ptr<iNet::ITcpSocket> socket)
        {
            ServerSessionCompletedFn fn = 
                boost::bind(&Server::OnSessionCompleted, this, _1, _2);

            m_sessions.Add( 
                new ServerSession(getMsgThread(), m_settings, socket, fn) 
                );
        }

    public:

        Server(iCore::MsgThread &thread, const SrvSettings &settings, CompletedFn onSrvError) : 
          iCore::MsgObject(thread),
          m_settings(settings),
          m_onSrvError(onSrvError),
          m_tcpServer(thread, this)
        {
            ESS_ASSERT(m_onSrvError);

            Utils::HostInf addr("127.0.0.1", 0);
            ESS_ASSERT( m_tcpServer.Listen(addr) == 0 );

            ClearDstDir();
        }

        Utils::HostInf Interface() const
        {
            return m_tcpServer.LocalHostInf();
        }
    };

}  // namespace

// -----------------------------------------------------------

namespace iCoro
{

    class CopyTask : public Utils::IVirtualDestroy
    {        
        CompletedFn m_callback;

        Server m_server;
        ClientPool m_clients; 

        void OnServerError(QString info)
        {
            ESS_ASSERT( info.size() );

            if (m_callback)
            {
                m_callback("Server error: " + info);            
                m_callback = CompletedFn();  // clear
            }
        }

        void OnClientsDone(QString info)
        {
            if ( info.size() ) info = "Client error: " + info;

            if (m_callback)
            {
                m_callback(info);
                m_callback = CompletedFn();  // clear
            }
        }

    public:

        CopyTask(iCore::MsgThread &thread, QString srcDir, QString dstDir, CompletedFn callback)  :
          m_callback(callback),
          m_server(thread,  SrvSettings(dstDir),                          boost::bind(&CopyTask::OnServerError, this, _1) ),
          m_clients(thread, ClientSettings(srcDir), m_server.Interface(), boost::bind(&CopyTask::OnClientsDone, this, _1) ) 
        {
        }

    };

    // ---------------------------------------------------

    void NObjCoroFtpTest::Run( DRI::IAsyncCmd *pAsyncCmd, QString srcDir, QString dstDir )
    {
        m_body.reset( 
            new CopyTask(getMsgThread(), srcDir, dstDir, boost::bind(&T::OnCompleted, this, _1) ) 
            );

        AsyncBegin(pAsyncCmd, boost::bind(&T::DoAbort, this) );
    }

}
