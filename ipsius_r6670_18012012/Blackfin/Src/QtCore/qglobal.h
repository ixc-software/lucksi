
#ifndef QGLOBAL_H
#define QGLOBAL_H

// -------------------------------------------------------------
// for QtCore

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

// -------------------------------------------------------------

namespace Qt
{
    enum CaseSensitivity 
    {
        CaseInsensitive,
        CaseSensitive
    };
    
} // namesapce Qt

// -------------------------------------------------------------
 
class QBool
{
    bool b;

public:
    inline explicit QBool(bool B) : b(B) {}
    inline operator const void *() const
    { return b ? static_cast<const void *>(this) : static_cast<const void *>(0); }
};

inline bool operator==(QBool b1, bool b2) { return !b1 == !b2; }
inline bool operator==(bool b1, QBool b2) { return !b1 == !b2; }
inline bool operator==(QBool b1, QBool b2) { return !b1 == !b2; }
inline bool operator!=(QBool b1, bool b2) { return !b1 != !b2; }
inline bool operator!=(bool b1, QBool b2) { return !b1 != !b2; }
inline bool operator!=(QBool b1, QBool b2) { return !b1 != !b2; }

// -------------------------------------------------------------

/*
#define Q_ASSERT(cond) do {if(!(cond))qt_assert(#cond,__FILE__,__LINE__);} while (0)
#define Q_ASSERT(cond) do{}while(0)
*/

// -------------------------------------------------------------
// for QtNetwork

typedef signed char qint8;         /* 8 bit signed */
typedef unsigned char quint8;      /* 8 bit unsigned */
typedef short qint16;              /* 16 bit signed */
typedef unsigned short quint16;    /* 16 bit unsigned */
typedef int qint32;                /* 32 bit signed */
typedef unsigned int quint32;      /* 32 bit unsigned */


#endif
