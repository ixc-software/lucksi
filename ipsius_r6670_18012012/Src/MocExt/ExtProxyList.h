#ifndef __EXTPROXY_LIST__
#define __EXTPROXY_LIST__

#include "ExtProxy.h"
#include "IMocExtension.h"

namespace MocExt
{
    class FunctionList;

	// Список структур ClassDef для которых необходимо генерация Proxy	
    class InterfacesProxyList : public IMocExtension
    {
        QList<InterfaceProxy> m_InterfacesList;

        void CreateListOfFunction(FunctionList &functionsList, const ClassDef &new_class);

        const ClassDef *SearchClass(const QByteArray &name) const;
        bool ClassIsRemoteInterface(const ClassDef &new_interface) const;

    // IMocExtension impl 
    private:

        void ProcessClass(const ClassDef &new_class, QString classNamespaces,
            /* out */ std::string &warning, QList<QByteArray> &includes);

        void GenerateFile(FILE *f) const;            

    public:

        /*
        void Append(const ClassDef &new_class);
        void Generate(FILE *out) const; */

    };  
	
} // namespace MocExt  

#endif
