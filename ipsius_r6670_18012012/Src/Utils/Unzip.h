#ifndef __UTILS_UNZIP__
#define __UTILS_UNZIP__

#include "ZipArchive.h"
#include "IBasicInterface.h"
#include "ErrorsSubsystem.h"

namespace Utils
{
    using Platform::byte;

    class IUnzipFileFilter : public Utils::IBasicInterface
    {
    public:
        virtual bool ExtractItem(ZipItemBase &item, QString itemName) = 0;
    };

    // ------------------------------------------

    struct Unzip : boost::noncopyable
    {

    public:

        static const char* CDefaultCodepage() { return "IBM 866"; }

        ESS_TYPEDEF(UnzipErr);
        ESS_TYPEDEF_FULL(CantCreatePath, UnzipErr);
        ESS_TYPEDEF_FULL(CantSaveFile, UnzipErr);
        ESS_TYPEDEF_FULL(FileExists, UnzipErr);
        ESS_TYPEDEF_FULL(BadCodecName, UnzipErr);

        static void DoUnzip(const QString &zipFileName, QString outputDir = "", 
            bool withOverwrite = true, QString fileNameCodec = CDefaultCodepage(), 
            IUnzipFileFilter *pFilter = 0)   // can throw ZipArchive::ZipException or I/O error
        {            
            if (!outputDir.isEmpty())  // fix outputDir
            {
                if (!IsDir(outputDir)) outputDir += QString("/");
            }

            std::vector<byte> fileBuff(64 * 1024);
            QString prevFilePath;

            const QTextCodec *pC = QTextCodec::codecForName(fileNameCodec.toStdString().c_str());
            if (pC == 0) 
			{
				ESS_THROW(BadCodecName);
				return;  // dummi
			}

            ZipArchive arch(zipFileName.toStdString());

            while(!arch.Eof())
            {
                ZipItemBase &item = arch.Current();
                QString itemName = pC->toUnicode( item.Name().c_str() );

                bool add = (pFilter == 0) ? true : pFilter->ExtractItem(item, itemName);

                if (add)
                {
                    QString fullName = outputDir + itemName;
                    ProcessItem(item, fullName, prevFilePath, fileBuff, withOverwrite);
                }

                // next
                arch.Next();
            }  

        }

        static bool IsDir(const QString &s)
        {
            return s.endsWith("\\") || s.endsWith("/");
        }

    private:

        Unzip() {}

        static void ProcessItem(ZipItemBase &item, QString fullName, QString &prevFilePath,
            std::vector<byte> &fileBuff, bool withOverwrite)
        {
            if (IsDir(fullName)) 
            {
                if (!QDir().mkpath(fullName)) 
                {
                    ESS_THROW_MSG(CantCreatePath, fullName.toStdString());
                }
            }
            else
            {
                QString path = QFileInfo(fullName).absolutePath();
                if (path != prevFilePath)
                {
                    QDir().mkpath(path);
                    prevFilePath = path;
                }

                if (!ExtractFile(fullName, item, fileBuff, withOverwrite)) 
                {
                    ESS_THROW_MSG(CantSaveFile, fullName.toStdString());
                }
            }
        }


        static bool ExtractFile(QString fileName, ZipItemBase &item,
            std::vector<byte> &fileBuff, bool withOverwrite)
        {
            if (!withOverwrite)
            {
                QFileInfo fi(fileName);
                if (fi.exists()) ESS_THROW(FileExists);
            }

            QFile f(fileName);
            if (!f.open(QIODevice::WriteOnly)) return false;
            f.resize(0);

            item.BeginRead();

            while(true)
            {
                bool eof = item.Read(fileBuff, fileBuff.capacity());

                if (fileBuff.size() > 0)
                {
                    int bytesWriten = f.write((char*)&fileBuff[0], fileBuff.size());
                    if (bytesWriten != fileBuff.size()) return false;
                }

                if (eof) break;
            }

            return true;
        }

    };

    // ------------------------------------------
    
    // extract all jpeg files (by format and extansions)
    void RunUnzipTest(QString zipFileName);

    
} // namespace Utils

#endif
