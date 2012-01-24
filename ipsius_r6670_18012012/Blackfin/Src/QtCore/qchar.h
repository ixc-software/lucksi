
#ifndef QCHAR_H
#define QCHAR_H

#include "qglobal.h"

class QChar
{
    ushort m_wch; // wchar_t
     
public:
    QChar(): m_wch(0) {}
    QChar(char c); 
    QChar(uchar c);

    QChar(uchar cell, uchar row) : m_wch((row << 8) | cell){}
    QChar(ushort rc): m_wch(rc) {}
    QChar(short rc): m_wch(ushort(rc)){}
    QChar(uint rc): m_wch(ushort(rc & 0xffff)){}
    QChar(int rc): m_wch(ushort(rc & 0xffff)){}

    enum SpecialCharacter 
    {
        Null = 0x0000,
        Nbsp = 0x00a0,
        ReplacementCharacter = 0xfffd,
        ObjectReplacementCharacter = 0xfffc,
        ByteOrderMark = 0xfeff,
        ByteOrderSwapped = 0xfffe,
        ParagraphSeparator = 0x2029,
        LineSeparator = 0x2028
    };
    
    QChar(SpecialCharacter s) : m_wch(ushort(s)) {}

    enum Category
    {
        NoCategory,

        Mark_NonSpacing,          //   Mn
        Mark_SpacingCombining,    //   Mc
        Mark_Enclosing,           //   Me

        Number_DecimalDigit,      //   Nd
        Number_Letter,            //   Nl
        Number_Other,             //   No

        Separator_Space,          //   Zs
        Separator_Line,           //   Zl
        Separator_Paragraph,      //   Zp

        Other_Control,            //   Cc
        Other_Format,             //   Cf
        Other_Surrogate,          //   Cs
        Other_PrivateUse,         //   Co
        Other_NotAssigned,        //   Cn

        Letter_Uppercase,         //   Lu
        Letter_Lowercase,         //   Ll
        Letter_Titlecase,         //   Lt
        Letter_Modifier,          //   Lm
        Letter_Other,             //   Lo

        Punctuation_Connector,    //   Pc
        Punctuation_Dash,         //   Pd
        Punctuation_Open,         //   Ps
        Punctuation_Close,        //   Pe
        Punctuation_InitialQuote, //   Pi
        Punctuation_FinalQuote,   //   Pf
        Punctuation_Other,        //   Po

        Symbol_Math,              //   Sm
        Symbol_Currency,          //   Sc
        Symbol_Modifier,          //   Sk
        Symbol_Other,             //   So

        Punctuation_Dask = Punctuation_Dash // oops
    };
    
    char toAscii() const;
    static QChar fromAscii(char c);

    char toLatin1() const { return (m_wch > 0xff) ? '\0' : char(m_wch); }
    static QChar fromLatin1(char c) { return QChar(c); }

    
    ushort unicode() const { return m_wch; }
    ushort& unicode() { return m_wch; }

    QChar toLower() const;
    QChar toUpper() const;
    bool isSpace() const;
    
};

inline bool operator==(QChar c1, QChar c2) { return c1.unicode() == c2.unicode(); }
inline bool operator!=(QChar c1, QChar c2) { return c1.unicode() != c2.unicode(); }
inline bool operator<=(QChar c1, QChar c2) { return c1.unicode() <= c2.unicode(); }
inline bool operator>=(QChar c1, QChar c2) { return c1.unicode() >= c2.unicode(); }
inline bool operator<(QChar c1, QChar c2) { return c1.unicode() < c2.unicode(); }
inline bool operator>(QChar c1, QChar c2) { return c1.unicode() > c2.unicode(); }


#endif
