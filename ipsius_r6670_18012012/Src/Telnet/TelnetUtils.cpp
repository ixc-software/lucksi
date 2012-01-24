
#include "stdafx.h"

#include "TelnetUtils.h"
#include "Utils/ErrorsSubsystem.h"

namespace 
{
    const std::string CDefaultCodecName = "Windows-1251";

    enum 
    {
        UnicodeCharSize = 2,
    };

    
} // namespace

// ----------------------------------------

namespace Telnet
{

    // ----------------------------------------

    string TypeConvertion::DefaultCodecName()
    {
        return CDefaultCodecName;
    }

    // ----------------------------------------
    /*
    static QByteArray TypeConvertion::ToQByteArray(const QString &data, 
                                   const std::string codacName = "Windows-1251")
    {
        QByteArray res = data.toLocal8Bit();

        // if unicode
        if (res.isNull())
        {
            QTextCodec *pC = QTextCodec::codecForName(codacName.c_str());
            if (pC != 0)
            { 
                pC->fromUnicode(data);
                //delete pC;
            }
        }

        return res;
    }
    */
    // ----------------------------------------

    QString TypeConvertion::ToAscii(const QByteArray &data, 
                           const string &codecName)
    {
        // if ascii
        if (codecName.empty()) return data.data();

        QTextCodec *pC = QTextCodec::codecForName(codecName.c_str());
        QString res = (pC != 0) ? pC->toUnicode(data) : QString(data);
        
        return res;
    }

    // ----------------------------------------

    QString TypeConvertion::ToUnicode(const QByteArray &data)
    {
        ESS_ASSERT(UnicodeCharSize != 0);
        ESS_ASSERT((data.size() % UnicodeCharSize) == 0);

        QString res;
        res.reserve(data.size() / UnicodeCharSize);

        int i = 0;
        while (i < data.size())
        {
            // QChar ( uchar cell, uchar row )
            res += QChar(data.at(i + 1), data.at(i));
            i += UnicodeCharSize;
        }

        return res;
    }

    // ----------------------------------------

    QString TypeConvertion::ToBinary(const QByteArray &data)
    {
        QString res; 
        res.reserve(data.size());

        for (int i = 0; i < data.size(); ++i)
        {
            res += data.at(i);
        }

        return res;
        //return QString(data);
    }

    // ----------------------------------------

    QByteArray TypeConvertion::FromAscii(const QString &data,
                                const string &codecName)
    {
        if (codecName.empty()) return data.toAscii();
        
        QTextCodec *pC = QTextCodec::codecForName(codecName.c_str());
        QByteArray res = (pC != 0) ? pC->fromUnicode(data) : data.toAscii();

        return res;
    }

    // ----------------------------------------

    QByteArray TypeConvertion::FromUnicode(const QString &data)
    {
        QByteArray res;
        res.reserve(data.size() * 2); 

        for (int i = 0; i < data.size(); ++i)
        {
            // uchar QChar::row () const
            // row (most significant byte) 
            // row == hi (?)
            res += data.at(i).row();
            res += data.at(i). cell();
        }

        return res;
    }

    // ----------------------------------------

    QByteArray TypeConvertion::FromBinary(const QString &data)
    {
        QByteArray res;
        res.reserve(data.size());

        for (int i = 0; i < data.size(); ++i)
        {
            res += data.at(i);
        }

        return res;
        //return data.toLocal8Bit();
    }

    // ----------------------------------------

    word TypeConvertion::ToWord(byte hi, byte lo)
    {
        word result = (static_cast<word>(hi) << 8) 
                     + (static_cast<word>(lo));
        return result;
    }

    // ----------------------------------------
    /*
    dword ToDword(const QByteArray &num)
    {
        dword res = 0;

        int size = (num.size() > 4) ? 4 : num.size();

        for (int i = size; i > 0; ++i)
        {
            res += (static_cast<dword>(At(num, i)) << (8 * i));
        }

        return res;
    }
    */
    // ----------------------------------------

    QByteArray TypeConvertion::ToQByteArray(word num)
    {
        byte hi = num >> 8;
        QByteArray result;
        result += static_cast<char>(hi);

        byte lo = num & 0xFF;
        result += static_cast<char>(lo);

        return result;
    }

} // namespace Telnet

