

#include "stdafx.h"
#include "qchar.h"

#include "qunicodetables_p.h"
#include "qunicodetables.cpp"


#define FLAG(x) (1 << (x))

// ----------------------------------------------------

QChar::QChar(char ch)
{
    /*if (QTextCodec::codecForCStrings())
    {
        m_wch =  QTextCodec::codecForCStrings()->toUnicode(&ch, 1).at(0).unicode();
    }
    else*/ 
        m_wch = uchar(ch);
}

// ----------------------------------------------------

QChar::QChar(uchar ch)
{
    /*if (QTextCodec::codecForCStrings()) 
    {
        char c = char(ch);
        m_wch =  QTextCodec::codecForCStrings()->toUnicode(&c, 1).at(0).unicode();
    } 
    else*/ 
        m_wch = ch;
}

// ----------------------------------------------------

char QChar::toAscii() const 
{
    /*if (QTextCodec::codecForCStrings())
    {
        return QTextCodec::codecForCStrings()->fromUnicode(QString(*this)).at(0);
    }
    return m_wch > 0xff ? 0 : char(m_wch);
    */
    
    return toLatin1();
}

// ----------------------------------------------------

QChar QChar::fromAscii(char c)
{
    /*if (QTextCodec::codecForCStrings())
    {
        return QTextCodec::codecForCStrings()->toUnicode(&c, 1).at(0).unicode();
    }
    return QChar(ushort((uchar)c));
    */

    return fromLatin1(c);
}

// ----------------------------------------------------

QChar QChar::toLower() const 
{
    const QUnicodeTables::Properties *p = qGetProp(m_wch);
    return (!p->lowerCaseSpecial)? (m_wch + p->lowerCaseDiff) : m_wch;
}

// ----------------------------------------------------

QChar QChar::toUpper() const 
{
    const QUnicodeTables::Properties *p = qGetProp(m_wch);
    return (!p->upperCaseSpecial)? (m_wch + p->upperCaseDiff) : m_wch;
}

// ----------------------------------------------------

bool QChar::isSpace() const
{
    return (m_wch >= 9 && m_wch <=13) ? true : false;

    /*
    const int test = FLAG(Separator_Space) |
                     FLAG(Separator_Line) |
                     FLAG(Separator_Paragraph);
    return FLAG(qGetProp(m_wch)->category) & test; */
}

// ----------------------------------------------------
