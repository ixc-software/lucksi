#include "stdafx.h"

#include "Unzip.h"

// ----------------------------------------------------------

namespace Utils
{
    
    void RunUnzipTest(QString zipFileName)
    {
        class Filter : public IUnzipFileFilter
        {
            bool ExtractItem(ZipItemBase &item, QString itemName)
            {
                if (Unzip::IsDir(itemName)) return true;

                itemName = itemName.toUpper();
                if (itemName.endsWith(".JPG") || itemName.endsWith(".JPEG")) return true;

                // format detection (fake, demo version)
                std::string buff;
                buff.resize(16);

                item.BeginRead();
                int bytesReaded;
                item.Read(&buff[0], buff.size(), bytesReaded);
                if (bytesReaded != buff.size()) return false;

                if (buff.substr(6, 4) == "JFIF") return true;

                return false;
            }
        };

        QString outputDir = QFileInfo(zipFileName).absolutePath() + "/ext";

        Filter f;
        Utils::Unzip::DoUnzip(zipFileName, outputDir, false, Unzip::CDefaultCodepage(), &f);
    }
    
    
}  // namespace Utils

