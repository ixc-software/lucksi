#ifndef QBYTEARRAY_H
#define QBYTEARRAY_H

#include <string>
#include "qglobal.h"
// #include "qstring.h"

class QString;

class QByteArray
{
    std::string m_data;
    
public:
    QByteArray() {}
    QByteArray(const char *data): m_data(data) {}
    QByteArray(const char *data, int size): m_data(data, size) {}
    QByteArray(int size, char c): m_data(size, c) {}
    QByteArray(const QByteArray &other): m_data(other.m_data) {}
    ~QByteArray() {}

    QByteArray& operator=(const QByteArray &other);
    QByteArray& operator=(const char *str);

    int size() const { return m_data.size(); }
    bool isEmpty() const { return m_data.empty(); }
    void resize(int size) { m_data.resize(size); }

    QByteArray& fill(char c, int size = -1);

    int capacity() const { return m_data.capacity(); }
    void reserve(int size) { m_data.reserve(size); }

    // char* data();
    const char* data() const { return m_data.c_str(); }
    inline const char* constData() const { return m_data.c_str(); }

    void clear() { m_data.clear(); }

    const char at(int i) const { return m_data.at(i); }
    char& operator[](int i);
    char& operator[](uint i);
    
    int indexOf(char c, int from = 0) const;
    int indexOf(const char *c, int from = 0) const;
    int indexOf(const QByteArray &a, int from = 0) const;

    int lastIndexOf(char c, int from = -1) const;
    int lastIndexOf(const char *c, int from = -1) const;
    int lastIndexOf(const QByteArray &a, int from = -1) const;
    
    
    QBool contains(char c) const { return QBool(indexOf(c) >= 0); }
    QBool contains(const char *a) const { return QBool(indexOf(a) >= 0); }
    QBool contains(const QByteArray &a) const { return QBool(indexOf(a) >= 0); }

    QByteArray left(int len) const;
    QByteArray right(int len) const;
    QByteArray mid(int index, int len = -1) const;

    bool startsWith(const QByteArray &a) const { return (indexOf(a) == 0); }
    bool startsWith(char c) const { return (indexOf(c) == 0); }
    bool startsWith(const char *c) const { return (indexOf(c) == 0); }

    bool endsWith(const QByteArray &a) const { return ( lastIndexOf(a) == ( size() - a.size() ) ); }

    bool endsWith(char c) const { return (lastIndexOf(c) == (size() - 1)); }
    bool endsWith(const char *c) const { return endsWith( QByteArray(c) ); }

    void truncate(int pos);
    void chop(int n);

    QByteArray trimmed() const;
    
    QByteArray& append(char c);
    QByteArray& append(const char *s);
    QByteArray& append(const QByteArray &a);
    
    QByteArray& insert(int i, char c);
    QByteArray& insert(int i, const char *s);
    QByteArray& insert(int i, const QByteArray &a);

    QByteArray& remove(int index, int len);

    QByteArray& replace(int index, int len, const char *s);
    QByteArray& replace(int index, int len, const QByteArray &s);

    QByteArray& operator+=(char c);
    QByteArray& operator+=(const char *s);
    QByteArray& operator+=(const QByteArray &a);

    typedef char *iterator;
    typedef const char *const_iterator;
    typedef iterator Iterator;
    typedef const_iterator ConstIterator;
    
    // iterator begin() { return data(); }
    const_iterator begin() const { return data(); }
    const_iterator constBegin() const { return data(); }
    // iterator end() { return ( data() + size() ); }
    const_iterator end() const { return ( data() + size() ); }
    const_iterator constEnd() const { return ( data() + size() ); }

    // stl compatibility
    typedef const char &const_reference;
    typedef char &reference;
    
    void push_back(char c) { m_data.push_back(c); }
    void push_back(const char *c);
    void push_back(const QByteArray &a) { push_back( a.data() ); }

    int count() const { return size(); }
    int length() const { return size(); }
    bool isNull() const { return isEmpty(); }
};

// --------------------------------------------------------------

inline bool operator==(const QByteArray &a1, const QByteArray &a2)
{ 
    return std::string(a1.data()) == std::string(a2.data());
}

// --------------------------------------------------------------

inline bool operator==(const QByteArray &a1, const char *a2)
{ 
    return operator==(a1, QByteArray(a2));
}

// --------------------------------------------------------------

inline bool operator==(const char *a1, const QByteArray &a2)
{ 
    return operator==(QByteArray(a1), a2);
}

// --------------------------------------------------------------

inline bool operator!=(const QByteArray &a1, const QByteArray &a2)
{ 
    return !operator==(a1, a2); 
}

// --------------------------------------------------------------

inline bool operator!=(const QByteArray &a1, const char *a2)
{ 
    return !operator==(a1, a2); 
}

// --------------------------------------------------------------

inline bool operator!=(const char *a1, const QByteArray &a2)
{ 
    return !operator==(a1, a2);
}

// --------------------------------------------------------------

inline bool operator<(const QByteArray &a1, const QByteArray &a2)
{ 
    return operator<( std::string(a1.data()), std::string(a2.data()) ); 
}

// --------------------------------------------------------------

inline bool operator<(const QByteArray &a1, const char *a2)
{ 
    return operator<(a1, QByteArray(a2)); 
}

// --------------------------------------------------------------

inline bool operator<(const char *a1, const QByteArray &a2)
{ 
    return operator<(QByteArray(a1), a2); 
}

// --------------------------------------------------------------


/*
    TODO:
    __________________________________________

    operator const char *() const;
    operator const void *() const;

    QByteRef operator[](int i);
    QByteRef operator[](uint i);

    int count(char c) const;
    int count(const char *a) const;
    int count(const QByteArray &a) const;

    QByteArray toLower() const;
    QByteArray toUpper() const;

    QByteArray& replace(char before, const char *after);
    QByteArray& replace(char before, const QByteArray &after);
    QByteArray& replace(const char *before, const char *after);
    QByteArray& replace(const QByteArray &before, const QByteArray &after);
    QByteArray& replace(const QByteArray &before, const char *after);
    QByteArray& replace(const char *before, const QByteArray &after);
    QByteArray& replace(char before, char after);

    QList<QByteArray> split(char sep) const;
  
    QByteArray &append(const QString &s);
    QByteArray &insert(int i, const QString &s);
    QByteArray &replace(const QString &before, const char *after);
    QByteArray &replace(char c, const QString &after);
    QByteArray &replace(const QString &before, const QByteArray &after);
    
    QByteArray &operator+=(const QString &s);
    int indexOf(const QString &s, int from = 0) const;
    int lastIndexOf(const QString &s, int from = -1) const;

    bool operator==(const QString &s2) const;
    bool operator!=(const QString &s2) const;
    bool operator<(const QString &s2) const;
    bool operator>(const QString &s2) const;
    bool operator<=(const QString &s2) const;
    bool operator>=(const QString &s2) const;

    short toShort(bool *ok = 0, int base = 10) const;
    ushort toUShort(bool *ok = 0, int base = 10) const;
    int toInt(bool *ok = 0, int base = 10) const;
    uint toUInt(bool *ok = 0, int base = 10) const;
    long toLong(bool *ok = 0, int base = 10) const;
    ulong toULong(bool *ok = 0, int base = 10) const;
    qlonglong toLongLong(bool *ok = 0, int base = 10) const;
    qulonglong toULongLong(bool *ok = 0, int base = 10) const;
    float toFloat(bool *ok = 0) const;
    double toDouble(bool *ok = 0) const;
    QByteArray toBase64() const;
    QByteArray toHex() const;

    QByteArray &setNum(short, int base = 10);
    QByteArray &setNum(ushort, int base = 10);
    QByteArray &setNum(int, int base = 10);
    QByteArray &setNum(uint, int base = 10);
    QByteArray &setNum(qlonglong, int base = 10);
    QByteArray &setNum(qulonglong, int base = 10);
    QByteArray &setNum(float, char f = 'g', int prec = 6);
    QByteArray &setNum(double, char f = 'g', int prec = 6);

    static QByteArray number(int, int base = 10);
    static QByteArray number(uint, int base = 10);
    static QByteArray number(qlonglong, int base = 10);
    static QByteArray number(qulonglong, int base = 10);
    static QByteArray number(double, char f = 'g', int prec = 6);
    static QByteArray fromRawData(const char *, int size);
    static QByteArray fromBase64(const QByteArray &base64);
    static QByteArray fromHex(const QByteArray &hexEncoded);

    void push_front(char c);
    void push_front(const char *c);
    void push_front(const QByteArray &a);
*/


#endif
