#include "ExtNamedObjectList.h"
#include "ExtMoc.h"

// ---------------------------------------------------

namespace MocExt
{
	const char *CNamedObjectInd = "NObj";
	const char *CNamedObject = "NamedObject";
	const char *CDomainNamedObject = "Domain::NamedObject";

	bool IsNamedObject(const QString &className)
	{
		return className.startsWith(QString::fromLocal8Bit(CNamedObjectInd));
	}

	bool IsCorrectNamedObject(const QString &className)
	{
		return (className.size() > sizeof(CNamedObjectInd) && 
			!className.at(sizeof(CNamedObjectInd)).isLower());	
	}

	bool SuperclassIsNamedObject(const ClassDef &new_interface)
	{
		int size = new_interface.superclassList.size();
		for(int i = 0; i < size; ++i)
		{
			const QPair<QByteArray, FunctionDef::Access> &superclass = new_interface.superclassList.at(i);

			if( superclass.first == CDomainNamedObject ||
				superclass.first == CNamedObject) return true;

			if(IsNamedObject(QString::fromLocal8Bit(superclass.first.constData())) && IsCorrectNamedObject(QString::fromLocal8Bit(superclass.first.constData()))) return true;
		}
		return false;
	}

};

namespace MocExt
{

    // ------------------------------------------

    bool NamedObjectList::SearchClass(const QString &name) const
    {
        return m_namedObjects.filter(name).size() != 0;
    }

    // ----------------------------------------------------------------

    void NamedObjectList::ProcessClass( const ClassDef &new_class, QString classNamespaces, 
        /* out */ std::string &warning, QList<QByteArray> &includes )
    {
        if (!new_class.hasQObject) return;

        QString name( QString::fromLocal8Bit(new_class.qualified.constData()) );

        name.remove(classNamespaces);
        if (!IsNamedObject(name)) return;

        if (!IsCorrectNamedObject(name))
        {
            warning = std::string ("Named object list append error. Incorrect name format. Class: ") +
                new_class.qualified.constData();
            return;
        }

        if (name.contains(QString::fromLocal8Bit("::")))  // it is inner class
        {
            warning = std::string ("Named object list append error. It is inner class. Class: ") +
                new_class.qualified.constData();
            return;
        }

        if (SuperclassIsNamedObject(new_class)) 
        {
            if (m_namedObjects.empty())
            {
                includes.push_back("DRI/NamedObjectFactory.h");
            }

            m_namedObjects.push_back( QString::fromLocal8Bit(new_class.qualified.constData()) );
        }

        return;
    }

    void NamedObjectList::GenerateFile( FILE *f ) const
    {
        if(!m_namedObjects.size()) return;

        QTextStream out(f);
        out << "\n";
        out << "// ---------------------------------------------- \n";
        out << "// NObj register \n";
        out << "\n";

        out << "namespace\n{\n";

        for(int i = 0; i < m_namedObjects.size(); ++i)
        {
            out << "\tDRI::NamedObjectTypeRegister<"
                << m_namedObjects.at(i) << ">" << " GReg_" << i << ";\n" ;
        }
        out << "}; // end namespace \n";
    }

  } // namespace MocExt

