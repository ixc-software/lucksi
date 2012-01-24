#ifndef __IMOCEXTENSION__
#define __IMOCEXTENSION__

#include <string>

namespace MocExt
{
    
    class IMocExtension
    {
    public:
        
        virtual void ProcessClass(const ClassDef &new_class, QString classNamespaces,
         /* out */ std::string &warning, QList<QByteArray> &includes) = 0;
            
        virtual void GenerateFile(FILE *f) const = 0;            
        
        virtual ~IMocExtension() {}                
    };
    
}  // namespace MocExt

#endif