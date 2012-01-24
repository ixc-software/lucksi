#ifndef __EXTQOBJECT__
#define __EXTQOBJECT__

#include "IMocExtension.h"

namespace MocExt
{

    class ExtQObject : public IMocExtension
    {
        QStringList m_list;

    // IMocExtension impl
    private:

        void ProcessClass(const ClassDef &new_class, QString classNamespaces,
            /* out */ std::string &warning, QList<QByteArray> &includes)
        {
            if (!new_class.hasQObject) return;

            QString name( QString::fromLocal8Bit(new_class.qualified.constData()) );

            name.remove(classNamespaces);

            if (name.contains(QString::fromLocal8Bit("::")))  // it is inner class -- skip it 
            {
                // problem: can't access to meta info defined in private meta classes 
                // TODO: implement thru direct access to meta info (defined in files upper) - ?
                return;  
            }

            if (m_list.empty())
            {
                includes.push_back( "Utils/QObjFactory.h" );  // TODO - fix!
            }

            m_list.push_back( QString::fromLocal8Bit(new_class.qualified.constData()) );
        }

        void GenerateFile(FILE *f) const
        {
            if (!m_list.size()) return;

            QTextStream out(f);
            out << "\n";
            out << "// ---------------------------------------------- \n";
            out << "// QObject register \n";
            out << "\n";

            out << "namespace\n{\n";

            for(int i = 0; i < m_list.size(); ++i)
            {
                out << "\tUtils::QObjFactoryRegister<"
                    << m_list.at(i) << ">" << " GRegQObj_" << i << ";\n" ;
            }
            out << "}; // end namespace \n";

        }

    };  

    
}  // namespace MocExt


#endif