
#include "stdafx.h"
#include "qtextcodec.h"


QTextCodec *QTextCodec::cftr = 0;

// ------------------------------------------------------------
/*
class QWindowsLocalCodec: public QTextCodec
{
public:
    QWindowsLocalCodec()
    {
    }

    ~QWindowsLocalCodec()
    {
    }

    QString convertToUnicode(const char *chars, int len) const
    {
        return qt_winMB2QString(chars, len);
    }
    
    QByteArray convertFromUnicode(const QChar *uc, int len) const
    {
        return qt_winQString2MB(uc, len);
    }
    
    QByteArray name() const
    {
        return "System";
    }
    
    int mibEnum() const
    {
        return 0;
    }

};
*/
// ------------------------------------------------------------


