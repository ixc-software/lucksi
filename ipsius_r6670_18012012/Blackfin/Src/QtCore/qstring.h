#ifndef QSTRING_H
#define QSTRING_H

#include <string>
#include "qchar.h"
#include "qbytearray.h"
#include "qglobal.h"

// -------------------------------------------
/*
    Now ASCII-convertion works as Latin1-convertion

    TODO: 
        - ASCII-convertion using QTextCodec
        - numeric convertion depends on 'base' parameter 
        - access operators witch return QChar& and QChar*

*/


// class QTextCodec;

class QString
{
    std::wstring m_data;
    // static QTextCodec *codecForCStrings;
    
public:

    QString() {}
    QString(const QChar *unicode, int size);
    QString(QChar c) { append(c); }
    QString(int size, QChar c) { fill(c, size); }
    QString(const QString &other) : m_data(other.m_data) {}
    QString(const char *ch);
    
    QString(const QByteArray &a) { *this = QString( a.data() ); }
    
    ~QString() {}
    
    QString& operator=(QChar c);
    QString& operator=(const QString &other);
    QString& operator=(const char *ch) { return (*this = fromAscii(ch)); }
    QString& operator=(char c) { return (*this = QChar::fromAscii(c)); }
    QString& operator=(const QByteArray &a) { return (*this = QString(a)); }
    
    int size() const { return m_data.size(); }
    int count() const { return size(); }
    int length() const { return m_data.length(); }
    bool isEmpty() const { return m_data.empty(); }
    void resize(int size) { m_data.resize(size); }

    QString& fill(QChar c, int size = -1); 
    void truncate(int pos) ;
    void chop(int n);

    int capacity() const { return m_data.capacity(); }
    void reserve(int size) { m_data.reserve(size); }
    void clear() { m_data.clear(); }

    int indexOf(QChar c, int from = 0, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    int indexOf(const QString &s, int from = 0, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    int lastIndexOf(QChar c, int from = -1, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    int lastIndexOf(const QString &s, int from = -1, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
   
    QBool contains(QChar c, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    QBool contains(const QString &s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    int count(QChar c, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    int count(const QString &s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

    QString left(int n) const;
    QString right(int n) const;
    QString mid(int position, int n = -1) const;

    bool startsWith(const QString &s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    bool startsWith(const QChar &c, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    bool endsWith(const QString &s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    bool endsWith(const QChar &c, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    
    QString toLower() const;
    QString toUpper() const;

    // const char toAscii() const;
    QByteArray toAscii() const;
    static QString fromAscii(const char *str, int size = -1);

    QByteArray toLatin1() const;
    static QString fromLatin1(const char *str, int size = -1);


    QString trimmed() const;
    
    QString& insert(int i, QChar c);
    QString& insert(int i, const QChar *uc, int len);
    QString& insert(int i, const QString &s);
    
    QString& append(QChar c);
    QString& append(const QString &s);
    QString& append(const char *s);
    QString& append(const QByteArray &s) { return append(QString(s)); }

    QString& operator+=(const char *s) { return append(QString::fromAscii(s)); }
    QString& operator+=(const QByteArray &s) { return append(QString(s)); }
    QString& operator+=(char c) { return append(QChar::fromAscii(c)); }
    QString& operator+=(QChar c) { return append(c); }
    QString& operator+=(QChar::SpecialCharacter c) { return append(QChar(c)); }
    QString& operator+=(const QString &s) { return append(s); }

    QString& remove(int i, int len);
    QString& remove(QChar c, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    QString& remove(const QString &s, Qt::CaseSensitivity cs = Qt::CaseSensitive);

    QString& replace(int i, int len, QChar after);
    QString& replace(int i, int len, const QChar *s, int slen);
    QString& replace(int i, int len, const QString &after);
    QString& replace(QChar before, QChar after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    QString& replace(QChar c, const QString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    QString& replace(const QString &before, const QString &after,
                     Qt::CaseSensitivity cs = Qt::CaseSensitive);

    /*
    enum SplitBehavior { KeepEmptyParts, SkipEmptyParts };

    QStringList split(const QString &sep, SplitBehavior behavior = KeepEmptyParts,
                      Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    QStringList split(const QChar &sep, SplitBehavior behavior = KeepEmptyParts,
                      Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

    QStringList split(const QRegExp &sep, SplitBehavior behavior = KeepEmptyParts) const;
    */

    
    int compare(const QString &s) const;
    int compare(const QString &s, Qt::CaseSensitivity cs) const;
    static int compare(const QString &s1, const QString &s2);
    static int compare(const QString &s1, const QString &s2, Qt::CaseSensitivity cs);
    
    bool operator==(const QString &s) const { return (m_data == s.m_data); }
    bool operator<(const QString &s) const { return (m_data < s.m_data); }
    bool operator>(const QString &s) const { return s < *this; }
    bool operator!=(const QString &s) const { return !operator==(s); }
    bool operator<=(const QString &s) const { return !operator>(s); }
    bool operator>=(const QString &s) const { return !operator<(s); }

    bool operator==(const char *s) const { return *this == QString(s); }
    bool operator!=(const char *s) const { return !operator==(s); }
    bool operator<(const char *s) const { return operator<( QString(s) ); }
    bool operator>(const char *s2) const { return operator>( QString(s2) ); }
    bool operator<=(const char *s2) const { return !operator>(s2); }
    bool operator>=(const char *s2) const { return !operator<(s2); }
    
    bool operator==(const QByteArray &s) const { return operator==(QString(s)); }
    bool operator!=(const QByteArray &s) const { return !operator==(s); }
    bool operator<(const QByteArray &s) const { return operator<(QString(s)); }
    bool operator>(const QByteArray &s) const { return operator>(QString(s)); }
    bool operator<=(const QByteArray &s) const { return operator<=(QString(s)); }
    bool operator>=(const QByteArray &s) const { return operator>=(QString(s)); }
    

    void push_back(QChar c) { append(c); }
    void push_back(const QString &s) { append(s); }
    
    int toInt(bool *ok = 0, int base = 10) const;
    uint toUInt(bool *ok = 0, int base = 10) const;
    
    QString& setNum(ushort num, int base = 10);
    QString& setNum(int num, int base = 10);
    static QString number(int num, int base = 10);

    const QChar at(int i) const { return m_data.at(i); }

    /*
    QChar* data(); 
    const QChar* unicode() const;
    const QChar* data() const;
    const QChar* const_data() const;
    */
    /*
    const QChar operator[](int i) const { return m_data[i]; }
    const QChar operator[](uint i) const { return m_data[i]; }
    QChar& operator[](int i); 
    QChar& operator[](uint i);
    */
    /*
    typedef QChar *iterator;
    typedef const QChar *const_iterator;
    typedef iterator Iterator;
    typedef const_iterator ConstIterator;
    */

    wchar_t& operator[](int i);
    wchar_t& operator[](uint i);

    const wchar_t* data() const { return m_data.data(); }
    const wchar_t* unicode() const { return data(); }
    const wchar_t* constData() const { return data(); }
    
    typedef wchar_t* iterator;
    typedef const wchar_t* const_iterator;
    typedef iterator Iterator;
    typedef const_iterator ConstIterator;

    // iterator begin() { return data(); }
    const_iterator begin() const { return constData(); }
    const_iterator constBegin() const { return begin(); }
    // iterator end() { return data() + size();  }
    const_iterator end() const { return ( constData() + size() ); }
    const_iterator constEnd() const { return end(); }

    static QString fromStdString(const std::string &s);
    std::string toStdString() const;

    static QString fromStdWString(const std::wstring &s);
    std::wstring toStdWString() const { return m_data; }

    struct Null { };
    static const Null null;
    QString(const Null &) {}
    QString &operator=(const Null &) { return (*this = QString()); }
    bool isNull() const { return isEmpty(); }
};

#endif
