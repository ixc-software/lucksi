
#include "stdafx.h"
#include "QtHelpers.h"

// ---------------------------------------------------

namespace
{
	unsigned char WinToDos(unsigned char b)
	{
		if ((b >= 0xC0) && (b <= 0xEF)) return(b - 0x40);
		if ((b >= 0xF0) && (b <= 0xFF)) return(b - 0x10);
		return(b);
	}
}


// ---------------------------------------------------

std::ostream& operator<< (std::ostream& out, const QString s)
{
    std::string outS = Utils::QStringToString(s);
	
	out << outS;

	return out;
}

// ---------------------------------------------------

namespace Utils
{

	std::wstring QStringToWString(const QString &s)
	{
		std::wstring res;
	
		for(int i = 0; i < s.size(); i++)
		{
			wchar_t c = s[i].unicode();
			res.push_back(c);
		}
	
		return res;
	}
	
	// ---------------------------------------------------
	
	std::string QStringToString(const QString &s, bool useOEM)
	{
		const QTextCodec *pC = QTextCodec::codecForName("Windows-1251");
		assert(pC);
	
		QByteArray resArr = pC->fromUnicode(s);
	
		std::string res;
	
		for(int i = 0; i < resArr.length(); i++)
		{
			char c = resArr[i];
			if (useOEM) c = WinToDos(c);
			res.push_back(c);
		}
	
		return res;
	}
	
	// ---------------------------------------------------
	
	QString WStringToQString(const std::wstring &s)
	{
	    return QString::fromStdWString(s);
	}
	
	// ---------------------------------------------------
	
	QString StringToQString(const std::string &s)
	{
	    return QString::fromStdString(s);
	}

    // ---------------------------------------------------

    bool QtLoadFile(QString fileName, QByteArray &result)
    {
        QFile file(fileName);

        if (!file.open(QIODevice::ReadOnly)) return false;

        result = file.readAll();

        return true;
    }

    // ---------------------------------------------------

    QString NumberFormat(Platform::int64 number)
    {
        QString s = QString::number(number);
        QString res;

        while(true)
        {
            if (s.size() > 3) 
            {
                res = QString("'%1%2").arg(s.right(3)).arg(res);
                s = s.left(s.size() - 3);
            }
            else
            {
                res = s + res;
                break;
            }
        }

        return res;
    }	
	
}  // namespace Utils


