
#ifndef __DRIPARSERCONSTS__
#define __DRIPARSERCONSTS__

#include "stdafx.h"

namespace DRI
{
    const QChar CPathSep = '.';
    const QChar COptInit = '=';
    const QChar CMethodInitBegin = '(';
    const QChar CMethodInitEnd = ')';
    const QChar CParamSep = ',';
    const QChar CCmdSep = ';';
    const QChar CStringChar = '"';

    const QString CComment                  = "//";
    const QString CGlobalCommentBegin       = "/*";
    const QString CGlobalCommentEnd         = "*/";

    // separator between:
    // - object name and params; 
    // - object's params
    const QChar CSep = ' '; 

    // all system symbols
    const QString CSystemSymbols = QString().append(CPathSep).append(COptInit)
                                            .append(CMethodInitBegin).append(CMethodInitEnd)
                                            .append(CParamSep).append(CCmdSep).append(CSep);
                                            
    // CS_xxx consts
    const QString CCSPrefix         = "CS_";
    const QString CCS_Begin         = "CS_Begin";
    const QString CCS_End           = "CS_End";
    const QString CCS_Discard       = "CS_Discard";
    const QString CCS_Print         = "CS_Print";

} // namespace DRI

#endif

