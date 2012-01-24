
#include "stdafx.h"

#include "Utils/ErrorsSubsystem.h"
#include "StringList.h"

// -------------------------------------------------------

namespace
{
    using namespace Utils;

    // sl.append( src[startIndx:lastIndx] );
    void AddString(StringList &sl, const QString &src, int startIndx, int lastIndx)
    {
        if (startIndx == lastIndx) 
        {
            sl.append("");
            return;
        }

        ESS_ASSERT(startIndx < src.size());

        ESS_ASSERT(startIndx < lastIndx);
        ESS_ASSERT(lastIndx <= src.size());

        sl.append( src.mid(startIndx, lastIndx - startIndx) );
    }

    // ------------------------------------------------------

    void ParseFromRawData(StringList &sl, void *pData, int dataSize, const char *pCodecName)
    {
        ESS_ASSERT(pCodecName != 0);
        const QTextCodec *pC = QTextCodec::codecForName(pCodecName);
        ESS_ASSERT(pC != 0);

        QString s = pC->toUnicode((char*)pData, dataSize);

        int stringStartPos = 0;
        int currPos = 0;

        while(currPos < dataSize)
        {
            int lfSize = 0; 

            // Linux LF
            if (s.at(currPos) == QChar(0x0a)) lfSize = 1;

            // DOS LF
            if (currPos + 1 < dataSize)
            {
                if (s.at(currPos) == QChar(0x0d) && s.at(currPos + 1) == QChar(0x0a))
                {
                    lfSize = 2;
                }
            }
            
            if (lfSize <= 0)  // no LF detected
            {
                currPos++;
            }
            else  // process LF
            {
                AddString(sl, s, stringStartPos, currPos);

                currPos += lfSize;
                stringStartPos = currPos;
            }

            // is it data end?
            if (currPos == dataSize)
            {
                AddString(sl, s, stringStartPos, currPos);
                break;
            }
            
        }  // while()
    }


}

// -------------------------------------------------------

namespace Utils
{

    StringList::StringList( void *pData, int dataSize, const char *pCodecName /*= "Windows-1251"*/ )
    {
        ParseFromRawData(*this, pData, dataSize, pCodecName);
    }
}
