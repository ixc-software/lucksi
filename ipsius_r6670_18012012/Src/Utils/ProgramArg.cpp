
#include "stdafx.h"

#include "ProgramArg.h"

namespace 
{
    using namespace Utils;

    // for test
    class ProgramArgTestClass
    {
        class Pair
        {
            QString m_raw;
            QString m_parsed;
            bool m_isWrong;
            QString m_joinElement;

            void CheckParsing()
            {
                QStringList res = ProgramArg::Parse(m_raw);

                TUT_ASSERT(res.join(m_joinElement) == m_parsed);
            }
        
            void CheckException()
            {
                bool wasException = false;
                try
                {
                    ProgramArg t(m_raw);
                }
                catch (ProgramArg::WrongCommandLineSyntax &e)
                {
                    wasException = true;
                }
        
                TUT_ASSERT(wasException);
            }

        public:
            Pair(const QString &raw, 
                 const QString &parsed, 
                 bool isWrong, 
                 const QString &joinElement)
            : m_raw(raw), m_parsed(parsed), m_isWrong(isWrong), m_joinElement(joinElement)
            {}

            void Check()
            {
                if (m_isWrong) CheckException();
                else CheckParsing();
            }
        };

        std::vector<Pair> m_pairs;

    public:
        ProgramArgTestClass()
        {
            AddTestPair("x=\"y y\"", "x=y y");
            AddTestPair("x \"x\"      ", "x|x");
            AddTestPair("\"x\"  x   x", "x|x|x");
            AddTestPair("x", "x");
            AddTestPair(" \"\"x\"\" ", "x");
            AddTestPair("\"x x x\" x \"x x x\"", "x x x|x|x x x");
            AddTestPair("x\\\"x", "x\"x");
            AddTestPair("x\\\\\"x x\"", "x\\x x");
            AddTestPair("\"\"\"x\"\"", "x");

            AddTestPair("", "", true);
        }

        void Run()
        {
            for (size_t i = 0; i < m_pairs.size(); ++i)
            {
                m_pairs.at(i).Check();
            }
        }

        void AddTestPair(const QString& raw, 
                         const QString &parsed, 
                         bool isWrong = false,
                         const QString &joinElement = QString('|'))
        {
            m_pairs.push_back(Pair(raw, parsed, isWrong, joinElement));
        }

    };

} // namespace

// ----------------------------------

namespace Utils
{
    // equal to boost::program_options::split_winmain, 
    // but also skip empty params
    QStringList ProgramArg::Parse(const QString &commandLine)
    {
        QStringList res;
        QString data(commandLine);

        // remove begin and end whitespaces
        data = data.trimmed();
        if (data.isEmpty()) return res;

        // parse
        QString current;
        bool inside_quoted = false;
        int backslash_count = 0; 
        for(int i = 0; i < data.size(); ++i)
        {
             if (data.at(i) == '"') 
             { 
                 // '"' preceded by even number (n) of backslashes generates 
                 // n/2 backslashes and is a quoted block delimiter 
                 if (backslash_count % 2 == 0) 
                 { 
                     current.append( QString(backslash_count / 2, '\\') ); 
                     inside_quoted = !inside_quoted; 
                     // '"' preceded by odd number (n) of backslashes generates 
                     // (n-1)/2 backslashes and is literal quote. 
                 } 
                 else 
                 {
                     current.append( QString(backslash_count / 2, '\\') );
                     current += '"';
                 } 

                 backslash_count = 0; 
             } 
             else 
                 if (data.at(i) == '\\') 
                 { 
                     ++backslash_count; 
                 }
                 else 
                 { 
                     // Not quote or backslash. All accumulated backslashes should be 
                     // added 
                     if (backslash_count) 
                     {
                         current.append( QString(backslash_count, '\\') ); 
                         backslash_count = 0; 
                     } 

                     if ( ( data.at(i).isSpace() ) && (!inside_quoted) ) 
                     { 
                         // Space outside quoted section terminate the current argument 
                         res << current; 
                         current.clear(); 
                     } 
                     else
                     {
                         current += data.at(i);
                     }

                 }
         } 

         // If we have trailing backslashes, add them 
         if (backslash_count) current.append( QString(backslash_count , '\\') );

         // If we have non-empty 'current' or we're still in quoted
         // section (even if 'current' is empty), add the last token.
         if (!current.isEmpty() || inside_quoted) res << current;

         // skip empty params
         res.removeAll("");

         return res;
    }
    
    // ----------------------------------

    void ProgramArg::ProgramArgTest()
    {
        ProgramArgTestClass test;
        test.Run();
    }

    // ----------------------------------
    /*
    namespace
    {
        void CheckPairCount(const QString &data, const QString &subStr)
        {
            if ((data.count(subStr) % 2) != 0)
            {
                ESS_THROW(ProgramArg::WrongCommandLineSyntax);
            }
        }
    
        // -----------------------------------------
    
        QString TrimQuotes(const QString &data)
        {
            if (data.indexOf('"') != 0) return data;
    
            QString res(data);
    
            while (true)
            {
                if ( (res.indexOf('"') != 0) 
                     || ( res.lastIndexOf('"') != (res.size() -1) ) ) break;
    
                else if ( res.lastIndexOf('"') == (res.size() - 1) )
                {
                    res = res.mid(1, (res.size() - 2));
                }
            }
            
            return res;
        }
    
        // -----------------------------------------
    
        void Add(QStringList &list, const QString &data)
        {
            if (data.isEmpty()) return;
    
            QString res(data);
    
            res = res.simplified();
            res = TrimQuotes(res);
    
            list << res;
        }

    } // namespace

    // Parser:
    // recognize spases(' ') as separators, except spaces inside double quotes;
    // remove spases from the begin and the end of the string;
    // remove multiple double quotes, for example, """ -> ";
    // throw "WrongCommandLineSyntax" exception if string contains odd quantity of '"';
    // skip empty params.
    QStringList ProgramArg::Parse(const QString &commandLine)
    {
        if (commandLine.isEmpty()) return QStringList();

        QString data(commandLine);
        data = data.simplified();
        
        QStringList res;
        int quoteCounter = 0;
        QString cell;

        int i = 0;
        while (i < data.size())
        {
            QChar current = data.at(i);

            if (current == '"') ++quoteCounter;

            if ( ( current == QChar(' ') ) && ( (quoteCounter % 2) == 0 ) )
            {
                Add(res, cell);

                cell.clear();
                quoteCounter = 0;
            }

            cell += current;
            ++i;
        }

        Add(res, cell);

        // accept only pair quotes
        if ( (quoteCounter % 2) != 0 ) ESS_THROW(WrongCommandLineSyntax);

        return res;
    }
    */

} // namespace Utils
