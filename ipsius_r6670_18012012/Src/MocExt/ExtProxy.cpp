#include "stdio.h"
#include "stdlib.h"
#include "moc.h"
#include "ExtOptions.h"
#include "ExtProxy.h"
#include "ExtProxyList.h"
#include "ExtFunction.h"
#include "Utils/CRC32.h"

namespace 
{
	template < class T> void OutputToHex(QTextStream &out, const T &t)
	{
		out << "0x" << hex << t << "UL" << dec;
	};
};


namespace MocExt
{

    void InterfaceProxy::Generate(FILE *file) const
    {
        //GenerateOld(file);
        GenerateNew(file);
    }

    // ------------------------------------------

    QByteArray InterfaceProxy::GenerateNamespaceName() const
    {
        QByteArray namespaceName = m_class.qualified;
        namespaceName.replace(':', '_');
        namespaceName = "\nnamespace " + namespaceName;
        return namespaceName;
    }
    
    // ------------------------------------------
    
    QByteArray InterfaceProxy::GenerateCrcString() const
    {
        QByteArray classCRC = m_class.qualified;
        m_FunctionsList.GenerateCrc(classCRC);
        return classCRC;
    }
    
    // ------------------------------------------

    void InterfaceProxy::GenerateOld(FILE *file) const
    {
        QTextStream out(file);
        {
            // begin namespace
            out << GenerateNamespaceName();				
            out << "\n{";
            out << "\n";
        }
        
        // enum generated
        m_FunctionsList.GenerateEnum(out);

        QByteArray classCRC = GenerateCrcString();
        Utils::CRC32  crc(GenerateCrcString());
       
        out << "\n";
        out << "\n    // CRC string '";
		OutputToHex(out, classCRC); 
		out << "'\n";

        // local proxy generated
        GenerateLocalProxyOld(out);
        
        out << "\n";        
        out << "\n    // ------------------------------------------";        
        out << "\n";        

        // remote proxy generated
        GenerateRemoteProxy(out);

        out << "\n";        
        out << "\n    // ------------------------------------------";        
        out << "\n";        

        // remote server generated    
        GenerateRemoteServer(out);
        
        out << "\n";
        out << "\n    Domain::RegisterMeta<" <<  m_class.qualified << ", LocalProxy, RemoteProxy, RemoteProxyHandle>";
        out << " RM(Utils::CRC32(";
		OutputToHex(out, crc.checksum());
		out << "));";

        // close namespace
        out << "\n\n}; // End of namespace";

        out << "\n";        
        out << "\n// -------------------------------------------------------";        
        out << "\n";        
    }

    // ------------------------------------------

    void InterfaceProxy::GenerateNew(FILE *file) const
    {
        QTextStream out(file);
        {
            // begin namespace
            out << GenerateNamespaceName() << "_New";				
            out << "\n{";
            out << "\n";
        }
		    
        out << "    using boost::shared_ptr;\n";

        if (m_class.qualified.contains("::"))
        {
            QByteArray className("::" + m_class.classname); 
            QByteArray namespaceName(m_class.qualified);

            namespaceName.replace(className, ";");
            
            out << "    using namespace " << namespaceName << '\n';

        }

        QByteArray classCRC = GenerateCrcString();
        Utils::CRC32  crc(GenerateCrcString());

        out << "\n    // CRC string '";
		OutputToHex(out, classCRC);
		out << "'\n";

        // local proxy generated
        GenerateLocalProxyNew(out);
        out << "\n";        
        out << "\n    ObjLink::RegisterObjectLinkMeta<TIntf, LocalIntfProxy>";
        out << " RM(Utils::CRC32(";
		OutputToHex(out, crc.checksum());
		out << "));";        
        // close namespace
        out << "\n\n}; // End of namespace\n";
    }

	
    // ------------------------------------------
    
    void InterfaceProxy::GenerateLocalProxyOld(QTextStream &out) const
    {
        // local proxy
        
		QByteArray localProxyClass = "LocalProxyClass";

		out << "\n    class " << localProxyClass << ':';
		out << "\n          public Domain::ProxyIntfLocal<" << m_class.qualified << ">,";
		out << "\n          public " << m_class.qualified;
		out << "\n    {";

		m_FunctionsList.GenerateLocalProxyOld(out);			

		// register meta        
		out << "\n    }; // End of local proxy";        

    }

	// ------------------------------------------

	void InterfaceProxy::GenerateLocalProxyNew(QTextStream &out) const
	{
		// local proxy

    	out << "\n    typedef  " << m_class.qualified << " TIntf;";
		out << '\n';
		out << "\n    class LocalIntfProxy : ";
		out << "\n        public iCore::MsgObject,";
		out << "\n        public TIntf";            
		out << "\n    {";                    
		out << "\n        typedef LocalIntfProxy T;";                    
		out << '\n';
		out << "\n        ObjLink::ILocalProxyHost &m_host;";                    
		out << "\n        TIntf *m_pI;";                    
		out << '\n';

		m_FunctionsList.GenerateLocalProxyNew(out);			

		out << "\n    public:";                            
		out << '\n';
		out << "\n        LocalIntfProxy(ObjLink::ILocalProxyHost &host)";
		out << "\n            : iCore::MsgObject(host.getMsgThreadForProxy()),";
		out << "\n            m_host(host)";
		out << "\n        {";
		out << "\n            m_pI = ObjLink::LocalProxyHostToInterface<TIntf>(host);";
		out << "\n        }";        
		out << '\n';      
		// register meta        
		out << "    \n}; // End of local proxy";        
	}

    // ------------------------------------------

    void InterfaceProxy::GenerateRemoteProxy(QTextStream &out) const
    {
        // remote proxy
        QByteArray remoteProxyClass = "RemoteProxyClass";

        out << "\n    class " << remoteProxyClass << ':';
        out << "\n          public Domain::ProxyIntfRemote";
        out << "\n          public " << m_class.qualified;
        out << "\n    {";
        out << "\n    public:"; 

        m_FunctionsList.GenerateRemoteProxy(out);
     
        out << "\n    }; // End of remote proxy";        
    }

    // ------------------------------------------

    void InterfaceProxy::GenerateRemoteServer(QTextStream &out) const
    {

        // server proxy
        out << "\n    class RemoteProxyHandle : public Domain::ProxyRemoteServerT<";
        out << m_class.qualified << ">";
        out << "\n    {";
        out << "\n    public:"; 

        // Generate constructor
        out << "\n        RemoteProxyHandle(Domain::IProxyServer *pSrv)";
        out << "\n            : Domain::ProxyRemoteServerT<";
        out << m_class.qualified << ">(pSrv)";
        out << "\n        {";
        out << "\n        }";
        // end of Generate constructor

        // Generate ProcessCall function
        out << "\n        void ProcessCall(Domain::RecvRemoteCall &call)  // override";
        out << "\n        {";
        out << "\n            Domain::RpcMethodID id = call.getRpcMethodID();";
        out << "\n            Domain::RpcMethodCRC crc = call.getRpcMethodCRC();";

        m_FunctionsList.GenerateServerProxy(out);

        out << "\n            OnBadMethodID(id);";
        out << "\n        }";
        
        // end of Generate ProcessCall function

        out << "\n    }; // End of server proxy";        
    }


} // namespace MocExt

