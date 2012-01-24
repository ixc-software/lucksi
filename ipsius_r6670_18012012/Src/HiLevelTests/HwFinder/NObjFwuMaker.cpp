#include "stdafx.h"
#include "NObjFwuMaker.h"

#include "Utils/ExeName.h"
#include "Utils/RangeList.h"

namespace 
{
    QString AbsPath(QString path, QString file)
    {    
        QDir dir(path);
        if (QDir::isRelativePath(path)) 
        {
            QString exeDir(Utils::ExeName::GetExeDir().c_str());
            path = exeDir + path;
        }
        if (!dir.exists()) dir.mkpath(path);            
        path.append("/").append(file);            
        return QDir::cleanPath(path);
    }       

    bool GenerateFakeLdr(QString fileName, int size)
    {
        QFile file(fileName);   
        if (!file.open(QIODevice::WriteOnly)) return false;
        if (file.size() != size) file.resize(size);
        return true;
    }        

    bool MakeFwuExt(int ver, QString numRange, QString type, QString mainLdr, QString fwuName)
    {
        QString pyModul = AbsPath("../../PyBootTest/src", "BootToolsMain.py");

        QString cmd =
            QString("python \"%1\" fwu_make /m \"%2\" /no_echo /version %3 /numbers \"%4\" /hw_id  \"%5\" /output \"%6\" /a")
            .arg(pyModul)
            .arg(mainLdr)
            .arg(ver)
            .arg(numRange)
            .arg(type)
            .arg(fwuName);

        int res = 
            system(cmd.toStdString().c_str());

        return res == 0; // ok
    }
} // namespace 

namespace HiLevelTests
{
    namespace HwFinder
    {    


        Q_INVOKABLE void NObjFwuMaker::MakeFwu( QString fwuName, int revision, QString boardTypeRange, QString boardNumRange )
        {
            if (getMainLdrFile().isEmpty()) ThrowRuntimeException("LdrFile is not set");
            if (fwuName.isEmpty()) ThrowRuntimeException("Fwu name is empty");  
            if (revision < 0) ThrowRuntimeException("Revision < 0");
            ValidateRangeSyntax(boardTypeRange);
            ValidateRangeSyntax(boardNumRange);

            //fwuName = QString("%1_%2").arg(revision).arg(fwuName);

            QString mainLdrAbs = AbsPath(m_ldrPath/*getLdrPath()*/, getMainLdrFile());            
            QString fwuNameAbs = AbsPath(m_FwuPath/*getFwuPath()*/, fwuName);            

            if (m_dummyLdrSize >= 0) 
            {
                if (!GenerateFakeLdr(mainLdrAbs, m_dummyLdrSize)) ThrowRuntimeException("Cant gen ldr");  ; 
            }

            if ( !MakeFwuExt(revision, boardNumRange, boardTypeRange, mainLdrAbs, fwuNameAbs) )
                ThrowRuntimeException("Cant create firmware");  

        }

        QString NObjFwuMaker::getMainLdrFile()
        {
            if (m_dummyLdrSize >= 0) return "DummyMain.ldr";
            return m_ldrFile;
        }

        void NObjFwuMaker::ValidateRangeSyntax( QString str )
        {
            Utils::ValueRange(str.toStdString(), "-");
            // cath(ParseError&){ThrowRuntime...}
        }

    } // namespace HwFinder
} // namespace HiLevelTests
