#include "moc.h"
#include "ExtMoc.h"
#include "ExtOptions.h"
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

    bool operator==(const FunctionDef &arg1, const FunctionDef &arg2)
    {
        int countArguments=arg1.arguments.count();

        if (countArguments != arg2.arguments.count() || 
            arg1.name != arg2.name || 
            arg1.type.name != arg2.type.name
           )          
        {
            return false;
        }

        for (int j = 0; j < countArguments; ++j) 
        {
            const ArgumentDef &a1 = arg1.arguments.at(j);
            const ArgumentDef &a2 = arg2.arguments.at(j);
            
            if (a1.normalizedType != a2.normalizedType) 
            {
                return false;
            }
        }
        return true;
    }

    // ------------------------------------------
    // Класс список функций. 

    void FunctionList::operator += (const QList<FunctionDef> &functionList)
    {
        for (int i = 0; i < functionList.count(); ++i) 
        {
            const FunctionDef &f = functionList.at(i);
            Append(f);
        }
    }

    //---------------------------------------------------------------
    // если функция отсутствует в списке, добавляем ее 
    void FunctionList::Append(const FunctionDef &m_FunctionDef)
    {
        for (int i = 0; i < m_FunctionsList.count(); ++i) 
        {
            const FunctionDef &f = m_FunctionsList.at(i);
            if (f == m_FunctionDef)
                return;                        
        };
        m_FunctionsList += (m_FunctionDef);            
    }
    //---------------------------------------------------------------
    
    bool FunctionList::checkFunctions() const
    {
        bool ret = 0;
        
        for (int i = 0; i < m_FunctionsList.count(); ++i) 
        {
            const Function &f = m_FunctionsList.at(i);
            ret = f.checkFunctions();			
        }
        
        return ret; 
    }

    bool FunctionList::Function::checkFunctions() const
    {
        bool errorInd = 0;
        if(!m_FunctionDef.isVirtual)
        {
//            QByteArray errorBuf;
//            errorBuf += "Function" + "\"" + m_FunctionDef.name + "\"" + "is not virtual";
        }    

		

		if(m_FunctionDef.type.name != "void")
        {
            QByteArray warningBuf;
            warningBuf += "Function \"";
            warningBuf += m_Signature;
            warningBuf += "\" is not void";
            ExtentionMoc::error(warningBuf.constData());
            errorInd = 1;
        }    

        if(m_FunctionDef.arguments.count() > 1)
        {
            QByteArray warningBuf;
            warningBuf += "Function \"";
            warningBuf += m_Signature;
            warningBuf += "\" has more then one argument";
            ExtentionMoc::error(warningBuf.constData());
            errorInd = 1;
        }    
        return !errorInd;

    }

    //---------------------------------------------------------------
    
    void FunctionList::GenerateLocalProxyOld(QTextStream &funcOut) const
    {
        funcOut << "\n        //--------------";
        funcOut << '\n';
        
        for (int i = 0; i < m_FunctionsList.count(); ++i) 
        {
            const Function &f = m_FunctionsList.at(i);
            f.GenerateLocalProxyOld(funcOut);			
            funcOut << "\n        //--------------";
            funcOut << '\n';
        }
    }

	//---------------------------------------------------------------

	void FunctionList::GenerateLocalProxyNew(QTextStream &funcOut) const
	{
		funcOut << "\n        //--------------";
		funcOut << '\n';

		for (int i = 0; i < m_FunctionsList.count(); ++i) 
		{
			const Function &f = m_FunctionsList.at(i);
			f.GenerateLocalProxyNew(funcOut);			
			funcOut << "\n        //--------------";
			funcOut << '\n';
		}
	}
	
    //---------------------------------------------------------------
    
    void FunctionList::GenerateRemoteProxy(QTextStream &out) const
    {
        for (int i = 0; i < m_FunctionsList.count(); ++i) 
        {
            const Function &f = m_FunctionsList.at(i);
            f.GenerateRemoteProxy(out);			
        }
    }
    
    //---------------------------------------------------------------
    
    void FunctionList::GenerateServerProxy(QTextStream &out) const   
    {
        for (int i = 0; i < m_FunctionsList.count(); ++i) 
        {
            const Function &f = m_FunctionsList.at(i);
            f.GenerateServerProxy(out);			
        }
    }
    

    //---------------------------------------------------------------

    void FunctionList::GenerateCrc(QByteArray &funcCRC) const
    {

        for (int i = 0; i < m_FunctionsList.count(); ++i) 
        {
            const Function &f = m_FunctionsList.at(i);
            f.GenerateCrcStr(funcCRC);
        }
    }

    //---------------------------------------------------------------

    void FunctionList::GenerateEnum(QTextStream &out) const
    {
        
        out << "\n     enum MethodID ";				
        out << "\n     {";

        for (int i = 0; i < m_FunctionsList.count(); ++i) 
        {
            const Function &f = m_FunctionsList.at(i);
            out << "\n          ";
            f.GenerateEnumIdMethod(out);			
            out << ',';					
        }

        out << "\n     }";

        out << "\n     enum MethodCRC";				
        out << "\n     {";

        for (int i = 0; i < m_FunctionsList.count(); ++i) 
        {
            const Function &f = m_FunctionsList.at(i);
            out << "\n         ";					
            f.GenerateEnumCrc(out);		
            out << ',';										
        }
        out << "\n     }";			
    }
    
    //---------------------------------------------------------------

    void FunctionList::Function::GenerateEnumIdMethod(QTextStream &out) const
    {
        out << "ID_" << m_FunctionDef.name;				
    }
    
    //---------------------------------------------------------------        
    
    void FunctionList::Function::GenerateEnumIdCrc(QTextStream &out) const
    {
        Utils::CRC32  crc(m_Signature);
        out << "CRC_" << m_FunctionDef.name;
        out << '_';
		OutputToHex(out, crc.checksum());
    }
    
    //---------------------------------------------------------------    

    void FunctionList::Function::GenerateEnumCrc(QTextStream &out) const
    {
        GenerateEnumIdCrc(out);
        Utils::CRC32  crc(m_Signature);
        out << " = ";
		OutputToHex(out, crc.checksum());
    }

    //---------------------------------------------------------------
    
    void FunctionList::Function::GenerateCrcStr(QByteArray &funcCRC) const
    {
        funcCRC += '/' +m_Signature;
    }
    
    //---------------------------------------------------------------

    void FunctionList::Function::GenerateSignature()
    {
        m_Signature = m_FunctionDef.type.name + ' '+ m_FunctionDef.name + '(';
        m_onSignature = m_FunctionDef.type.name + " On"+ m_FunctionDef.name + '(';

        QByteArray signature;
        for (int j = 0; j < m_FunctionDef.arguments.count(); ++j) 
        {
            const ArgumentDef &a = m_FunctionDef.arguments.at(j);
            if (j) 
            {
                signature += ",";
                m_Arguments += ",";
            }
            signature += a.normalizedType + ' ' + a.name;
            m_Arguments += a.name;
        }

        signature += ')';
        m_Signature += signature;
        m_onSignature += signature;
    }

    //---------------------------------------------------------------

    void FunctionList::Function::GenerateLocalProxyOld(QTextStream &out) const
    {
		out << "\n        " + m_Signature;
		out << "\n        {";
		out << "\n            if (!CheckConnected()) return;";
		out << '\n';
		out << "\n            getInterface()->" << m_FunctionDef.name << '('+m_Arguments+");";
		out << "\n        }\n";
    }

	//---------------------------------------------------------------

    /*
	void FunctionList::Function::GenerateLocalProxyNew(QTextStream &out) const
	{
		out << "\n        " << m_onSignature;
		out << "\n        {";
		out << "\n            if(!m_host.ConnectedForProxy()) return;";
		out << '\n';
		out << "\n            m_pI->" << m_FunctionDef.name << '('+m_Arguments+");";
		out << "\n        }";
		out << '\n';

		out << "\n        " << m_Signature;
		out << "\n        {";
		out << "\n            PutMsg(this, &T::" << "On" << m_FunctionDef.name;
		if(m_Arguments.size())
			out << ", " << m_Arguments;
		out << ");";
		out << "\n        }";
		out << '\n';
	} */

    void FunctionList::Function::GenerateLocalProxyNew(QTextStream &out) const
    {
        out << "\n        " << m_Signature;
        out << "\n        {";
        out << "\n            if(!m_host.ConnectedForProxy()) return;";
        out << "\n            m_host.SendMsgToDestinationObject( ";

        out << "CreateMsg(m_pI, &TIntf::" << m_FunctionDef.name;
        if(m_Arguments.size()) out << ", " << m_Arguments;
        out << ")";

        out << " );";  // SendMsgToDestinationObject
        out << "\n        }";
        out << '\n';
    }

	
    //---------------------------------------------------------------

    void FunctionList::Function::GenerateRemoteProxy(QTextStream &out) const
    {
        out << "\n        " + m_Signature;
        out << "\n        {";
        out << "\n            if (!CheckConnected()) return;";
        out << '\n';
        out << "\n            Domain::RemoteCall rc(getRemoteHandle(), ";
        GenerateEnumIdMethod(out);			
        out << ", ";														
        GenerateEnumIdCrc(out);							
        out << ");";										
        out << "\n        }\n";				
    }
    
    //---------------------------------------------------------------
    
    void FunctionList::Function::GenerateServerProxy(QTextStream &out) const
    {
        out << "\n";
        out << "\n            if (id == ";
        GenerateEnumIdMethod(out);
        out << ')';
        out << "\n            {";

        out << "\n                if (crc != ";
        GenerateEnumIdCrc(out);
        out << ')';
        out << "\n                {";
        out << "\n                    OnMethodBadCRC(\"";
        out << m_FunctionDef.name << "\", ";
        GenerateEnumIdMethod(out);
        out << ", ";
        GenerateEnumIdCrc(out);
        out << ", crc);";
        out << "\n                    return;";
        out << "\n                }";

        switch(m_FunctionDef.arguments.count())
        {
        case 0:
            out << "\n                getInterface()->";
            out << m_FunctionDef.name << "();";
        break;
        case 1:
        {
            const ArgumentDef &a = m_FunctionDef.arguments.at(0);
            QByteArray type = a.normalizedType;
            if(type.contains("boost::shared_ptr"))
            {
                out << "\n                " << type << " param(new";
                type.replace("boost::shared_ptr", " ");
                type.replace("<", " ");
                type.replace(">", " ");
                out << type << ");"; 
            }
            else if (type.contains("shared_ptr"))
			{
				out << "\n                " << type << " param(new";
				type.replace("shared_ptr", " ");
				type.replace("<", " ");
				type.replace(">", " ");
				out << type << ");"; 
			}
			else
			{
				if(type.endsWith('&'))
					type.chop(1);
				out << "\n                " << type << " param;"; 
			}
            
            out << "\n                call.io() >> param;";
            out << "\n                getInterface()->";
            out << m_FunctionDef.name << "(param);";

        }
        break;
        default:
            //"Error: function has More then one argument";
            return;
        }
        out << "\n                return;";
        out << "\n            }";				
    }

} // namespace MocExt

