#include "ExtProxyList.h"
#include "ExtProxy.h"
#include "ExtMoc.h"
// ---------------------------------------------------

namespace MocExt
{

//    const char *CIntfBaseClass = "Domain::IRemoteInterface";
    
    const char *CIntfBaseClass = "IObjectLinkInterface";
    const char *CIntfBaseClassWithNamespace = "ObjLink::IObjectLinkInterface";
    
    // ------------------------------------------

    const ClassDef *InterfacesProxyList::SearchClass(const QByteArray & name) const
    {
        for (int i = 0; i < m_InterfacesList.size(); ++i) 
        {
            const ClassDef &ClassDef = m_InterfacesList.at(i).GetClassDef();
            if(ClassDef.qualified.endsWith(name))
                return &ClassDef;
        }
        return 0;
    }

    // ------------------------------------------

    bool InterfacesProxyList::ClassIsRemoteInterface(const ClassDef &new_interface) const 
    {
        int size = new_interface.superclassList.size();
        if(size != 1) return false;
        
        const QPair<QByteArray, FunctionDef::Access> &superclass = new_interface.superclassList.at(0);
        return superclass.first == CIntfBaseClass ||
            superclass.first == CIntfBaseClassWithNamespace ||
            SearchClass(superclass.first);
    }

    void InterfacesProxyList::CreateListOfFunction(FunctionList &functionsList, const ClassDef &new_class)
    {
        functionsList += new_class.publicList;

        for (int i = 0; i < new_class.superclassList.size(); ++i) 
        {

            const QPair<QByteArray, FunctionDef::Access> &superclass = new_class.superclassList.at(i);
            const ClassDef *def = SearchClass(superclass.first);

            if(def)
                functionsList += def->publicList;        

        }
    }

    // ------------------------------------------

    void InterfacesProxyList::ProcessClass( const ClassDef &new_class, QString classNamespaces, 
        /* out */ std::string &warning, QList<QByteArray> &includes )
    {
        if (!ClassIsRemoteInterface(new_class)) return;

        FunctionList functionsList;
        CreateListOfFunction(functionsList, new_class);

        if (!functionsList.checkFunctions()) return;

        InterfaceProxy interface(functionsList, new_class); 
        m_InterfacesList += interface;
    }

    // ------------------------------------------

    void InterfacesProxyList::GenerateFile( FILE *f ) const
    {
        if(!m_InterfacesList.size()) return;

        {
            QTextStream out(f);
            out << "\n";
            out << "// --------------------------------------------------- \n";
            out << "// Interface code \n";
            out << "\n";

        }

        for (int i = 0; i < m_InterfacesList.size(); ++i) 
        {
            const InterfaceProxy &interface = m_InterfacesList.at(i); 
            interface.Generate(f);
        }
    }

} // namespace MocExt

