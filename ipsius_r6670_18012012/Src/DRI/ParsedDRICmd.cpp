
#include "stdafx.h"

#include "ParsedDRICmd.h"
#include "DRIParserHelpers.h"
#include "DRIParserConsts.h"
#include "driutils.h"
#include "Domain/Identificator.h"

namespace 
{
    const bool CDebugMode = false;

    // -------------------------------------------------

    using namespace DRI;

    bool ContainsUnquotedSystemSymbols(const QString &data)
    {
        int quoteCount = 0;
        for (int i = 0; i < data.size(); ++i)
        {
            QChar ch = data.at(i);
            if (ch == CStringChar) ++quoteCount;
            if ( ( CSystemSymbols.contains(ch) ) && ( (quoteCount % 2) == 0 ) ) return true;
        }

        return false;
    }

    // -------------------------------------------------

    /*QString UnquoteString(const QString &s)
    {
        int size = s.size();

        if (size >= 2)
        {
            if ( (s.at(0) == CStringChar) && (s.at(size - 1) == CStringChar))
            {
                return s.mid(1, size - 2);
            }
        }

        return s;
    }*/

    // -------------------------------------------------

    QString ActionTypeToQString(ParsedDRICmd::ActionType type)
    {
        QString res;

        switch (type)
        {
        case (DRI::ParsedDRICmd::Unknown) : 
            res = "Unknown";
            break;

        case DRI::ParsedDRICmd::MethodCall :
            res = "MethodCall";
            break;

        case DRI::ParsedDRICmd::PropertyWrite :
            res = "PropertyWrite";
            break;

        default:
            ESS_ASSERT(0 && "Undefined ActionType"); 
        }
        
        return res;
    }


} // namespace


// ------------------------------------------------------

namespace DRI
{
    ParsedDRICmd::ParsedDRICmd(const QString &cmd) : 
        m_originalCmd(cmd),
        m_cmd(cmd), 
        m_isRelativePath(false), 
        m_actType(Unknown)
    {
        ESS_ASSERT( !cmd.isEmpty() );
        Parse();
    }

    // -------------------------------------------------------

    ParsedDRICmd::ParsedDRICmd(bool isRelativePath, 
                               const QStringList &fullObjectName, 
                               const QString &methodOrPropertyName, 
                               const QStringList &params,
                               ActionType actType) : 
        m_isRelativePath(isRelativePath), 
        m_fullObjectName(fullObjectName), 
        m_methodOrPropertyName(methodOrPropertyName), 
        // m_params(params), 
        m_actType(actType)
    {
        ESS_ASSERT( !methodOrPropertyName.isEmpty() );
        if (params.size() > 1) ESS_ASSERT(actType == MethodCall);

        // unquote params copy
        for(int i = 0; i < params.size(); ++i)
        {
            QString pm = DriUtils::UnquoteString(params.at(i), CStringChar);
            m_params.push_back(pm);
        }
        
        /*
        if ( fullObjectName.isEmpty() ) 
        {
            ThrowErrorMsg("No method's or property's name found", methodOrPropertyName);
        }

        if (params.size() > 1) && (actType != MethodCall)
        {
            QString err = "Invalid ActionType for number of params > 1, should be ";
            err += ActionTypeToQString(MethodCall);
            ThrowErrorMsg(err);
        }
        */
    }

    // -------------------------------------------------------

    void ParsedDRICmd::Parse()
    {
        /*
            [.][ObjName[.ObjName2.]...]MethodName[ param1[, param2]...]
            [.][ObjName1[.ObjName2.]...]OptionName [= value]
        */

        if (CDebugMode) std::cout << "cmd = \'" << m_cmd.toStdString() << '\'' << std::endl;

        m_cmd = m_cmd.trimmed();

        CutNameAndPath();
        MakeParamList();
    }

    // -------------------------------------------------------

    void ParsedDRICmd::FillNameAndPath(const QString &data)
    {
        if (CDebugMode) std::cout << "FillNameAndPath" << std::endl;
        
        QString tmp(data);

        if ( tmp.startsWith(CPathSep) )
        {
            m_isRelativePath = true;
            tmp = tmp.remove(0, 1);
        }

        // check name validation
        if ( ( tmp.isEmpty() ) || ( tmp.endsWith(CPathSep) ) ) 
        {
            ThrowErrorMsg("No method's or property's name found", tmp);
        }

        QStringList nameAndPath = tmp.split(CPathSep, QString::KeepEmptyParts);
        // check syntax
        for (int i = 0; i < nameAndPath.size(); ++i)
        {
            QString name( nameAndPath.at(i) );
            if ( ( name.isEmpty() ) || ( !ValidateObjName(name) ) )
            {
                ThrowErrorMsg("Invalid method's or property's name or path", tmp);
            }
        }
        
        m_methodOrPropertyName = nameAndPath.last();
        nameAndPath.removeLast();
        m_fullObjectName = nameAndPath;
    }

    // -------------------------------------------------------

    void ParsedDRICmd::CutNameAndPath()
    {
        QStringList res;

        if (CDebugMode) std::cout << "CutNameAndPath" << std::endl;
        
        // 1) try split using CMethodInitBegin
        SplitWithQuotesConsideration(m_cmd, CMethodInitBegin, res);
        if (res.size() > 2) ThrowSepErrorMsg("Extra", CMethodInitBegin);
        if (res.size() == 1) 
        {
            // 2) try split using COptInit
            SplitWithQuotesConsideration(m_cmd, COptInit, res);
            if (res.size() > 2) ThrowSepErrorMsg("Extra", COptInit);
            if (res.size() == 1)
            {
                // 3) try split using CSep
                SplitWithQuotesConsideration(m_cmd, CSep, res);
            }
        }

        // fill fields
        if (res.size() < 1) ThrowErrorMsg("No method's or property's name found", m_cmd);
        QString nameAndPath = res.at(0).trimmed();
        FillNameAndPath(nameAndPath);

        // cut
        m_cmd = m_cmd.remove( 0, nameAndPath.size() );
    }

    // -------------------------------------------------------

    void ParsedDRICmd::SetParams(const QStringList &params)
    {
        if (CDebugMode) std::cout << "params: " << params.join(";").toStdString() << std::endl;

        m_params.clear(); // ?!

        // check and set params
        for (int i = 0; i < params.size(); ++i)
        {
            QString pm( params.at(i) );

            if ( pm.isEmpty() )
            {
                ThrowErrorMsg( "Missing method's parameter", params.join(CParamSep) );
            }

            // pm can also be unquoted path: obj1.obj11
            if ( (!ValidateObjName(pm) ) 
                 &&( ContainsUnquotedSystemSymbols(pm) ) )
            {
                QString err = "Parameter contains unquoted system symbol \'" + pm + "\'";
                ThrowErrorMsg(err, params.join(CParamSep));
            }

            pm = DriUtils::UnquoteString(pm, CStringChar);

            m_params.push_back(pm);
        }
    }

    // -------------------------------------------------------

    void ParsedDRICmd::MakeParamList()
    {
        // [ ][(]param[,] param2[)]
        // [ ][=]param
         
        if (CDebugMode) std::cout << "MakeParamList" << std::endl;

        m_cmd = m_cmd.trimmed();

        QStringList params;
       
        // if it's option's parameter, which starts with COptInit
        if ( m_cmd.startsWith(COptInit) )
        {
            m_cmd = m_cmd.remove(0, 1).trimmed();
            if ( m_cmd.isEmpty() ) ThrowSepErrorMsg("Missing property's parameter after", 
                                                        COptInit);
            // even is there is too many params will be syntax exception
            SetParams( QStringList(m_cmd) );
            m_actType = PropertyWrite;
            return;
        }

        // if there are method's parameters, 
        // which start with CMethodInitBegin and end with CMethodInitEnd
        if ( m_cmd.startsWith(CMethodInitBegin) )
        {
            if ( !m_cmd.endsWith(CMethodInitEnd) )
            {
                ThrowSepErrorMsg("Parameter's list does not end with", CMethodInitEnd);
            }
            // remove separators
            m_cmd = m_cmd.mid(1, m_cmd.size() - 2);
            m_actType = MethodCall;
        }

        // try split with using CParamSep
        if ( m_cmd.trimmed().endsWith(CParamSep) )
        {
            ThrowErrorMsg("Missing property's parameter after", CParamSep);
        }
        SplitWithQuotesConsideration(m_cmd, CParamSep, params, false);
        if (params.size() > 1) 
        {
            SetParams(params);
            m_actType = MethodCall;
            return;
        }

        // try split with using CSep
        SplitWithQuotesConsideration(m_cmd, CSep, params);
        SetParams(params);
        if (params.size() > 1) m_actType = MethodCall;
    }

    // -------------------------------------------------------

    void ParsedDRICmd::ThrowSepErrorMsg(const QString &whatWithSep, QChar sep)
    {
        // QString msg = whatWithSep + " \'" + sep + "\': \"" + m_cmd + "\"";
        QString msg = QString(":ERROR DRI::InvalidCommandSyntax: "
                              "%1 \'%2\': \"%3\"").arg(whatWithSep, sep, m_cmd);
        ESS_THROW_MSG( InvalidCommandSyntax, msg.toStdString() );
    }

    // -------------------------------------------------------

    void ParsedDRICmd::ThrowErrorMsg(const QString &what, const QString &cause)
    {
        if (CDebugMode) std::cout << "ThrowErrorMsg" << std::endl;
        
        QString msg(":ERROR DRI::InvalidCommandSyntax: ");
        msg += what;
        if ( !cause.isEmpty() ) msg += QString(": \"%1\"").arg(cause);
        if ( !m_originalCmd.isEmpty()) msg += QString(" in line: %1").arg(m_originalCmd);

        ESS_THROW_MSG( InvalidCommandSyntax, msg.toStdString() ); // <<----------
    }

    // -------------------------------------------------------

    QString ParsedDRICmd::ToQString() const
    {
        QString res;
        res += (m_isRelativePath) ? "Path: " : "AbsolutePath: ";
        res += m_fullObjectName.join(".");
        res += "\n";
        res += "Name: ";
        res += m_methodOrPropertyName;
        res += "\n";
        res += "Params: ";
        res += m_params.join(";");
        res += "\n";
        if (!m_postprocChanges.isEmpty())
        {
            res += "Params changes: ";
            res += m_postprocChanges;
            res += "\n"; 
        }
        res += "ActionType: ";
        res += ActionTypeToQString(m_actType);
        res += "\n";
        res += "----------\n";

        return res;
    }

    // -------------------------------------------------------

    bool ParsedDRICmd::operator==(const ParsedDRICmd &other) const
    {
        bool res = (m_actType == other.m_actType) 
                    && (m_fullObjectName == other.m_fullObjectName)
                    && (m_isRelativePath == other.m_isRelativePath)
                    && (m_methodOrPropertyName == other.m_methodOrPropertyName)
                    && (m_params == other.m_params)
                    && (m_cmd == other.m_cmd)
                    && (m_postprocChanges == other.m_postprocChanges);
        return res;
    }

    // -------------------------------------------------------

    QString ParsedDRICmd::ProcessedCmd() const
    {
        QString res = m_originalCmd;
        if (!m_postprocChanges.isEmpty()) res += QString(" // %2").arg(m_postprocChanges);
        
        return res;
    }

    // -------------------------------------------------------

    bool ParsedDRICmd::ValidateObjName(const QString &name)
    {
        return Domain::Identificator::CheckObjectName(name);
    }

} // namespace DRI

