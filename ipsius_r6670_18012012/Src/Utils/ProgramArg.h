#ifndef __PROGRAMARG__
#define __PROGRAMARG__

#include "Utils/ErrorsSubsystem.h"

namespace Utils
{
    class ProgramArg
    {
        QString m_name;
        QStringList m_params;

    public:

        ESS_TYPEDEF(WrongCommandLineSyntax);

        ProgramArg(int argc, char* argv[])
        : m_name(argv[0])
        {
            for (int i = 1; i < argc; ++i)
            {
                m_params << QString(argv[i]);
            }
        }

        ProgramArg(const QString &name, const QStringList &params)
        : m_name(name), m_params(params)
        {}

        ProgramArg(const QString &nameAndParams)
        {
            QStringList data(Parse(nameAndParams));

            // no program name
            if (data.isEmpty()) ESS_THROW(WrongCommandLineSyntax);

            m_name = data.at(0);

            data.removeFirst();
            m_params = data;
        }

        ProgramArg(const QString &name, const QString &params)
        : m_name(name), m_params(Parse(params))
        {}

        const QString& getName() const { return m_name; }
        const QStringList& getParams() const { return m_params; }
        const QString& getParam(int index) const { return m_params.at(index); }
        // const QString& operator[](int index) const { return m_params.at(index); }
        int ParamCount() const { return m_params.size(); }

        // Parser:
        // recognize spases(' ') as separators, except spaces inside double quotes;
        // remove spases from the begin and the end of the string;
        // remove multiple double quotes;
        // skip empty params.
        static QStringList Parse(const QString &commandLine);

        static void ProgramArgTest();
    };

} // namespace Utils

#endif
