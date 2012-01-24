
#ifndef __PARSEDDRICMD__
#define __PARSEDDRICMD__

#include "Utils/ErrorsSubsystem.h"
#include "Utils/SafeRef.h"

#include "DRIParserConsts.h"

namespace DRI
{
    // Parse raw command and store it.
    class ParsedDRICmd
        : boost::noncopyable
    {
    public:

        enum ActionType
        {
            Unknown,
            PropertyWrite,
            MethodCall,
        };

        ESS_TYPEDEF(InvalidCommandSyntax);

        ParsedDRICmd(const QString &cmd); // can throw InvalidCommandSyntax

        ParsedDRICmd(bool isRelativePath, const QStringList &fullObjectName, 
                     const QString &methodOrPropertyName, const QStringList &params,
                     ActionType actType);

        // is non-root
        bool IsRelativePath() const { return m_isRelativePath; }

        // return list of object names without first '.' if it was found out;
        // to gues belonging of those name(root or not) call IsRelativePath()
        const QStringList& getFullObjName() const { return m_fullObjectName; }

        QString FullObjName() const
        {
            QString res;

            for(int i = 0; i < m_fullObjectName.size(); ++i)
            {
                res += m_fullObjectName.at(i);
                if (i != (m_fullObjectName.size() - 1)) res += CPathSep;
            }

            return res;
        }

        const QString& getMethodOrPropertyName() const { return m_methodOrPropertyName; }
        const QStringList& getParams() const { return m_params; }
        int getParamsSize() const { return m_params.size(); }
        ActionType getActionType() const { return m_actType; }

        QString ToQString() const;
        bool operator==(const ParsedDRICmd &other) const;

        QString LocationInfo() const { return m_locationInfo; }
        void LocationInfo(QString val) { m_locationInfo = val; }

        QString OriginalCmd() const { return m_originalCmd; }

        QString ProcessedCmd() const;

    private:

        const QString m_originalCmd;
        QString m_locationInfo;
        QString m_cmd;  // unnessessary global var

        // parsed data
        bool m_isRelativePath; 
        QStringList m_fullObjectName;
        QString m_methodOrPropertyName;
        QStringList m_params;
        ActionType m_actType;
        QString m_postprocChanges;

        void Parse();
        void CutNameAndPath();
        void MakeParamList();
        void FillNameAndPath(const QString &data);
        void SetParams(const QStringList &params);
        void ThrowSepErrorMsg(const QString &whatWithSep, QChar sep);
        void ThrowErrorMsg(const QString &what, const QString &cause = QString::null);
        
        static bool ValidateObjName(const QString &name);
    };

} // namespace DRI

#endif

