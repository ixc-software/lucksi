#ifndef __NOBJBINTOCPP__
#define __NOBJBINTOCPP__

#include "Domain/NamedObject.h"
#include "Utils/StringList.h"
#include "Utils/ExeName.h"
#include "Utils/IntToString.h"
#include "Utils/QtHelpers.h"

namespace DriUtils
{
    using Domain::NamedObject;
    using Domain::IDomain;
    using Domain::ObjectName;
    
    class NObjBinToCpp : public NamedObject
    {
        Q_OBJECT;

        static QString AbsolutePath(QString path)
        {
            QDir d(path);
            
            if (d.isAbsolute()) return path;

            QString fullPath = Utils::ExeName::GetExeDir().c_str();
            fullPath += path;
            return QDir(fullPath).absolutePath();
        }

        static QString BinaryBytes(QByteArray data)
        {
            const int CBytesInLine = 16;

            QString res;
            QString line;

            for(int i = 0; i < data.size(); ++i)
            {
                std::string v = Utils::IntToHexString(Platform::byte(data.at(i)), true, 1);
                line += QString(v.c_str());
                if (i != data.size() - 1) line += ", ";

                if ((i + 1) % CBytesInLine == 0)
                {
                    res += line;
                    res += "\n";
                    line.clear();
                }
            }

            if (!line.isEmpty()) res += line;

            return res;
        }

    public:

        NObjBinToCpp(IDomain *pDomain, const ObjectName &name) :
          NamedObject(pDomain, name)
        {
        }

        Q_INVOKABLE void Make(QString binFileName, QString outputFileName) const
        {
            QByteArray binData; 
            if ( !Utils::QtLoadFile( AbsolutePath(binFileName), binData )) 
            {
                ThrowRuntimeException("Can't load binary file!");
            }

            Utils::StringList sl;

            QString templ = "#include \"stdafx.h\"\n#include \"Platform/Platform.h\"\n\n// %0%\n\nnamespace\n{\n\n    const Platform::byte CBinary[%1%] = {\n%2%\n    };    \n\n}  // namespace\n\n";

            QString dt = QDateTime::currentDateTime().toString();
            QString info = QString("File %1 @ %2")
                .arg(binFileName)
                .arg(dt);

            QString size = QString("%1").arg(binData.size());
            QString arr = BinaryBytes(binData);

            templ = templ.replace("%0%", info);
            templ = templ.replace("%1%", size);
            templ = templ.replace("%2%", arr);
            sl.append(templ);

            sl.SaveToFile( AbsolutePath(outputFileName) );
        }
                        
    };
    
    
}  // namespace DriUtils

// Template (replace LF and ")
/*

#include "stdafx.h"
#include "Platform/Platform.h"

// %0%

namespace
{

    const Platform::byte CBinary[%1%] = {
%2%
    };    

}  // namespace

*/


#endif
