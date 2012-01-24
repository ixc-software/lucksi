#ifndef QTEXTCODEC_H
#define QTEXTCODEC_H

#include "qstring.h"

/*
    TODO:
     - codecForName()

*/

class QTextCodec
{
public:
    static QTextCodec* codecForName(const QByteArray &name)
    {
        // ...
        return 0;
    }
    
    static QTextCodec* codecForName(const char *name) 
    { 
        return codecForName(QByteArray(name)); 
    }

    static QTextCodec* codecForCStrings()
    {
        return QString::codecForCStrings; 
    }
    
    static void setCodecForCStrings(QTextCodec *c)
    {
        QString::codecForCStrings = c; 
    }

    QString toUnicode(const QByteArray&) const
    {
        return convertToUnicode(a.constData(), a.length(), 0);
    }
    
    QString toUnicode(const char* chars) const
    {
        std::string str(chars);
        return convertToUnicode(chars, str.size(), 0);
    }
    
    QByteArray fromUnicode(const QString& uc) const
    {
        return convertFromUnicode(str.constData(), str.length(), 0);
    }
    
    struct ConverterState {};

    QString toUnicode(const char *in, int length, ConverterState *state = 0) const
    { 
        return convertToUnicode(in, length, state); 
    }
    
    QByteArray fromUnicode(const QChar *in, int length, ConverterState *state = 0) const
    {
        return convertFromUnicode(in, length, state); 
    }

protected:
    virtual QString convertToUnicode(const char *in, int length, ConverterState *state) const = 0;
    virtual QByteArray convertFromUnicode(const QChar *in, int length, ConverterState *state) const = 0;

    QTextCodec();
    virtual ~QTextCodec();

private:
    static QTextCodec *cftr;
};



#endif
