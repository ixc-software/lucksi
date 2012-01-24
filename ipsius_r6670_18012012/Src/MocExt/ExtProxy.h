#ifndef __EXTPROXY__

#define  __EXTPROXY__

#include "ExtFunction.h"
#include "moc.h"

namespace MocExt
{
    class InterfacesProxyList;

    // ---------------------------------------------------------------
    //  class InterfacesProxy  - генератор Proxy для конкретного ClassDef
    class InterfaceProxy
    {
    public:
        
        InterfaceProxy(const FunctionList &functionsList, const ClassDef &classDef) 
            : m_FunctionsList(functionsList), m_class(classDef)	
        {
        }
        
        // генерируется Proxy    
        void Generate(FILE *file) const;
        
        const ClassDef &GetClassDef() const 
        {
            return m_class;
        }

        InterfaceProxy& operator = (const InterfaceProxy &src)
        {
            m_FunctionsList = src.m_FunctionsList; 
            m_class = src.m_class;    
        }
        

    private:
        FunctionList m_FunctionsList;
        ClassDef m_class;
        
        QByteArray GenerateNamespaceName() const;
        QByteArray GenerateCrcString() const;        
        
        void GenerateOld(FILE *file) const;
        void GenerateNew(FILE *file) const;

        QByteArray GenerateNamespace() const;
        void GenerateLocalProxyOld(QTextStream &out) const;
        void GenerateLocalProxyNew(QTextStream &out) const;		
        void GenerateRemoteProxy(QTextStream &out) const;
        void GenerateRemoteServer(QTextStream &out) const;
    
    }; // class InterfaceProxy

} // namespace MocExt   

#endif

