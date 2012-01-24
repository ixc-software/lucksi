#ifndef __QTHELPERS__
#define __QTHELPERS__

#include "stdafx.h"

#include "Platform/PlatformTypes.h"

namespace Utils
{
	
	std::wstring QStringToWString(const QString &s);
	std::string QStringToString(const QString &s, bool useOEM = false);
	QString WStringToQString(const std::wstring &s);
	QString StringToQString(const std::string &s);

    // Функтор для Utils::Converter<>. 
    // Двунаправленное преобразование QString <--> std::string.
    template<class TOut>
    class QStd
    {
    public:
        
        TOut operator()(const QString& src)
        {
            return QStringToString(src);
        }

        TOut operator()(const std::string& src)
        {
            return StringToQString(src);
        }
    };

    bool QtLoadFile(QString fileName, QByteArray &result);

    // output like "12'456"
    QString NumberFormat(Platform::int64 number);
			
}  // namespace Utils


// must be in global namespace as QString type
std::ostream& operator<< (std::ostream& out, const QString s);


#endif

