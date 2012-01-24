
#include "stdafx.h"
#include "StringListTest.h"
#include "Utils/StringList.h"

namespace 
{
    using namespace Utils;
   
    void Check(const QString &toCheckStr, 
               const QString &codec, 
               const QStringList &shouldBe, 
               bool silentMode)
    {
        StringList toCheck(toCheckStr.toAscii().data(), 
                           toCheckStr.size(), 
                           codec.toAscii().data());

        if (!silentMode)
        {
            std::cout << "ToCheck str: " << toCheckStr.toStdString() << std::endl
                << "ToCheck: " << toCheck.join("|").toStdString() << std::endl
                << "ShouldBe: " << shouldBe.join("|").toStdString() << std::endl;
        }
        
        TUT_ASSERT(toCheck == shouldBe);
    }
    
} // namespace 

// -----------------------------------------------------

namespace UtilsTests
{
    /* 
    StringList(void *pData, int dataSize, const char *pCodecName = "Windows-1251"); 
    */
    void StringListTest(bool silentMode)
    {
        const QString linLF('\x0a');
        const QString winLF("\x0d\x0a");
        QString codec("Windows-1251");
        QStringList data;
        data << "qwerty" << "uiop[]" << "asdfg" << "hjkl;'";

        // empty data
        Check(QString(), codec, QStringList(), silentMode);

        // linux data
        Check(data.join(linLF), codec, data, silentMode);

        // win data
        Check(data.join(winLF), codec, data, silentMode);

        // combined
        {
            QString s;
            for (int i = 0; i < data.size(); ++i)
            {
                QString sep = ((i % 2) == 0)? linLF : winLF;

                s += data.at(i);
                if (i != (data.size() - 1)) s += sep;
            }
            Check(s, codec, data, silentMode);
        }

        if (!silentMode) std::cout << "Utils::StringList test: OK" << std::endl;
    }
    
} // namespace UtilsTests
