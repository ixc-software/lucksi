#ifndef _EXT_NAMED_OBJECT_LIST_H_
#define _EXT_NAMED_OBJECT_LIST_H_

#include "moc.h"
#include "IMocExtension.h"

namespace MocExt
{

    class NamedObjectList : public IMocExtension
    {
        bool SearchClass(const QString &name) const;
        QStringList m_namedObjects;

    // IMocExtension impl
    private:

        void ProcessClass(const ClassDef &new_class, QString classNamespaces,
            /* out */ std::string &warning, QList<QByteArray> &includes);

        void GenerateFile(FILE *f) const;            

    public:

        /*
        std::string Append(const ClassDef &new_class, QString classNamespaces);
        void Generate(FILE *out) const;
        int Size() const 
        {
            return m_namedObjects.size();
        } */

    };  
	
} // namespace MocExt 

#endif
