
#include "stdafx.h"

#include "Utils/Random.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/QtHelpers.h"

#include "DRI/DRICommandStreamParser.h"
#include "DRI/ParsedDRICmd.h"
#include "DRI/DRIParserHelpers.h"
#include "DRIParserTests.h"

#include "Domain/DomainTestRunner.h"
#include "Domain/DomainClass.h"

#include "Platform/PlatformTypes.h"

namespace
{
    const bool CDebug = false;
    
    // ----------------------------------------------

    using namespace DRI;

    QStringList sl(const QString &data = QString::null)
    {
        if ( data.isEmpty() ) return QStringList();
        
        return QStringList(data);
    }

    // ----------------------------------------------

    // Checks parsing in DRICommandStreamParser and ParsedDRICmd.
    class CheckTable 
    {
        DRICommandStreamParser m_parser;
        QString m_toCheck;
        int m_excCount;
        bool m_noException;

        boost::ptr_list<ParsedDRICmd> m_correctDataList;

   private:
        void CheckData()
        {
            if ( !m_parser.HasParsedCmd() ) return;

            boost::shared_ptr<ParsedDRICmd> cmd = m_parser.GetParsedCmd();

            if (CDebug) std::cout << cmd->ToQString().toStdString();

            TUT_ASSERT( !m_correctDataList.empty() );

            ParsedDRICmd& corrCmd = m_correctDataList.front();

            TUT_ASSERT( cmd->getFullObjName() == corrCmd.getFullObjName() );
            TUT_ASSERT( cmd->getMethodOrPropertyName() == corrCmd.getMethodOrPropertyName() );
            TUT_ASSERT( cmd->getParams() == corrCmd.getParams() );
            TUT_ASSERT( cmd->IsRelativePath() == corrCmd.IsRelativePath() );
            TUT_ASSERT( cmd->getActionType() == corrCmd.getActionType() );
            
            m_correctDataList.pop_front();
        }

    public:
        CheckTable(Domain::IDomain &domain, 
                   const QStringList &toCheck, int exceptionCount = 0) : 
            m_toCheck( toCheck.join("\n") ), 
            m_excCount(exceptionCount), m_noException( (exceptionCount == 0) )
        {
        }

        void AddCheckLine(const QStringList &fullObjectName, 
                          const QString &methodOrPropertyName, 
                          const QStringList &params, 
                          bool isRelativePath, 
                          ParsedDRICmd::ActionType actType)
        {
            //ParsedDRICmd(isRelativePath, fullObjectName, methodOrPropertyName,
            //             params,actType)
            m_correctDataList.push_back( 
                new ParsedDRICmd(isRelativePath, fullObjectName,
                                 methodOrPropertyName, params, actType) );
        }

        void Check()
        {
            if (CDebug) std::cout << "Exception counter(max) = " << m_excCount << std::endl;
            
            // split m_toCheck to parts and send to parser 
            int seed = QDateTime::currentDateTime().time().second() 
                       + QDateTime::currentDateTime().time().msec();
            Utils::Random random(seed);
            int commandPartLenth = 8; // do not change to bigger

            int i = 0;
            while ( i <= m_toCheck.size() )
            {
                int val = random.Next(commandPartLenth) + 1;
                int size = ((m_toCheck.size() - i) > val)? val : m_toCheck.size();
                try
                {
                    QString data = m_toCheck.mid(i, size);

                    if (CDebug) std::cout << "Add: " << data << std::endl;
                    
                    m_parser.AddData(data);
                    CheckData();
                }
                catch(ParsedDRICmd::InvalidCommandSyntax &e)
                {
                    --m_excCount;

                    // it's ok if only toCheck data was wrong
                    if (m_noException) 
                    {
                        std::cout << e.getTextMessage() << std::endl;
                        TUT_ASSERT(0 && "Test failed!");
                    }

                    if (CDebug) std::cout << ">> " << e.getTextMessage() << std::endl 
                                    << "Exception counter = " << m_excCount << std::endl;
                }

                i += size; // val;
            }

            if (CDebug) std::cout << "Finally: excCount = " << m_excCount << std::endl;
            ESS_ASSERT(m_excCount == 0); // catch all exceptions that suppose to be
        }
    };

    // ----------------------------------------------

    void SplitingDataAndOkNameParsingTest(Domain::IDomain &domain)
    {
        // check parsing '\n' and CCmdSep (ok)
        // check parsing ok-names
        QStringList rawOk = QStringList() 
                            << "path1.subpath1.susbsubpath1.name1"
                            << " .path2.subpath2.name2      ;  "
                            << " path3.name3 \"param31\nparam32\n\""
                            << ";.path_4.name_4; _path5._name5 \"; ; ; param5\";"
                            << "path6.name6 (params6) ; path7.name7 ;"
                            << ";;;;"
                            << "p\nath8.name8"
                            << "path9.nam\ne9"
                            << ";\n"
                            << ".nam e13.opt13\n"
                            << ".name14\n";

        CheckTable tOk(domain, rawOk);

        QStringList fullName1 = QStringList() << "path1" << "subpath1" << "susbsubpath1";
        QStringList fullName2 = QStringList() << "path2" << "subpath2";

        // AddCheckLine(path, name, params, isRootFlag, actType);
        tOk.AddCheckLine(fullName1,    "name1",     sl(),  false, ParsedDRICmd::Unknown);
        tOk.AddCheckLine(fullName2,    "name2",     sl(),  true,  ParsedDRICmd::Unknown);
        tOk.AddCheckLine(sl("path3"),  "name3",     sl("\"param31\nparam32\n\""), 
                                                           false, ParsedDRICmd::Unknown);
        tOk.AddCheckLine(sl("path_4"), "name_4",    sl(),  true,  ParsedDRICmd::Unknown);
        tOk.AddCheckLine(sl("_path5"), "_name5",    sl("\"; ; ; param5\""),
                                                           false, ParsedDRICmd::Unknown);
        tOk.AddCheckLine(sl("path6"),  "name6",     sl("params6"), 
                                                           false, ParsedDRICmd::MethodCall);
        tOk.AddCheckLine(sl("path7"),  "name7",     sl(),  false, ParsedDRICmd::Unknown);
        tOk.AddCheckLine(sl(),         "p",         sl(),  false, ParsedDRICmd::Unknown);
        tOk.AddCheckLine(sl("ath8"),   "name8",     sl(),  false, ParsedDRICmd::Unknown);
        tOk.AddCheckLine(sl("path9"),  "nam",       sl(),  false, ParsedDRICmd::Unknown);
        tOk.AddCheckLine(sl(),         "e9",        sl(),  false, ParsedDRICmd::Unknown);
        tOk.AddCheckLine(sl(),         "nam",       sl("e13.opt13"),  
                                                           true, ParsedDRICmd::Unknown);
        tOk.AddCheckLine(sl(),         "name14",    sl(),  true, ParsedDRICmd::Unknown);
        
        tOk.Check();
    }

    // ----------------------------------------------

    void WrongNamesParsingTest(Domain::IDomain &domain)
    {
        // no spaces in name
        QStringList rawWrong1 = QStringList() 
                             << ".name1.name11..opt1=1;"         // missing name111
                             << "..name21.opt2=2;"               // missing name2
                             << "3name.opt3 3;"                  // invalid name syntax
                             << "(some_data5);"                  // missing method name
                             << ".(some_data6);"                 // missing method name
                             << ".name7.\"fake_subname7\".opt7"  // invalid name syntax
                             << "na,me8.opt8;"                   // invalid name syntax
                             << "n\tame9.opt9;"                  // invalid name syntax
                             << "\".fake_name10.fake_opt10\""    // missing opt or method name
                             << ". name12 . name121 . opt12 \n"; // unquoted sys symb in param
                             
        QStringList rawWrong2 = QStringList() << " = 4;\n";      // missing opt name1
        QStringList rawWrong3 = QStringList() << ". \n";         // missing opt or method name;
        QStringList rawWrong4 = QStringList() << "\'\n";         // invalid name syntax
        QStringList rawWrong5 = QStringList() << ". name15\n";   // missing name
        QStringList rawWrong6 = QStringList() << "name16.\n";    // missing name

        CheckTable tWrong1( domain, rawWrong1, rawWrong1.size() );
        tWrong1.Check();

        CheckTable tWrong2( domain, rawWrong2, rawWrong2.size() );
        tWrong2.Check();

        CheckTable tWrong3( domain, rawWrong3, rawWrong3.size() );
        tWrong3.Check();

        CheckTable tWrong4( domain, rawWrong4, rawWrong4.size() );
        tWrong4.Check();

        CheckTable tWrong5( domain, rawWrong5, rawWrong5.size() );
        tWrong5.Check();

        CheckTable tWrong6( domain, rawWrong6, rawWrong6.size() );
        tWrong6.Check();
    }

    // ----------------------------------------------

    void OkParamsParsingTest(Domain::IDomain &domain)
    {
       QStringList rawOk = QStringList()
                            << "optOrMethod1"
                            << "optOrMethod2 1"
                            << ".opt3 =  \t\t 1 ;"
                            << "opt4 = \"prm\";"
                            << "opt5 = \'p\';"
                            << ".method6 (1 \"str\" "")"
                            << "method7(1, 2,3 \t)"
                            << "method8 1 \t \"2, 3\" "
                            << "method9 \"1, 2\", 3;"
                            << "method10 \";\"  \",\"  \"=\"  \" \" "
                            << "method11 ( )"
                            << "optOrMethod12 >?<|\\/~!@#$%^&*_+\n";

        CheckTable tOk( domain, rawOk );
        QStringList params6 = QStringList() << "1" << "\"str\"";
        QStringList params7 = QStringList() << "1" << "2" << "3";
        QStringList params8 = QStringList() << "1" << "\"2, 3\"";
        QStringList params9 = QStringList() << "\"1, 2\"" << "3";
        QStringList params10 = QStringList() << "\";\"" << "\",\"" << "\"=\"" << "\" \"";

        // AddCheckLine(path, name, params, isRootFlag, actType);
        tOk.AddCheckLine(sl(), "optOrMethod1",  sl(),          false, ParsedDRICmd::Unknown);
        tOk.AddCheckLine(sl(), "optOrMethod2",  sl("1"),       false, ParsedDRICmd::Unknown);
        tOk.AddCheckLine(sl(), "opt3",          sl("1"),       true,  ParsedDRICmd::PropertyWrite);
        tOk.AddCheckLine(sl(), "opt4",          sl("\"prm\""), false, ParsedDRICmd::PropertyWrite);
        tOk.AddCheckLine(sl(), "opt5",          sl("'p\'"),    false, ParsedDRICmd::PropertyWrite);
        tOk.AddCheckLine(sl(), "method6",       params6,       true,  ParsedDRICmd::MethodCall);
        tOk.AddCheckLine(sl(), "method7",       params7,       false, ParsedDRICmd::MethodCall);
        tOk.AddCheckLine(sl(), "method8",       params8,       false, ParsedDRICmd::MethodCall);
        tOk.AddCheckLine(sl(), "method9",       params9,       false, ParsedDRICmd::MethodCall);
        tOk.AddCheckLine(sl(), "method10",      params10,      false, ParsedDRICmd::MethodCall);
        tOk.AddCheckLine(sl(), "method11",      sl(),          false, ParsedDRICmd::MethodCall);
        tOk.AddCheckLine(sl(), "optOrMethod12", sl(">?<|\\/~!@#$%^&*_+"),     
                                                               false, ParsedDRICmd::Unknown);
        tOk.Check();
    }

    // ----------------------------------------------

    void WrongParamsParsingTest(Domain::IDomain &domain)
    {
        // "method(1, , , 2, 3)";
        // (), = , ' ', combinations
        // diff number of ' ', '\t'
        // ...
        
        QStringList rawWrong = QStringList()
                             << "opt1 = 1 1"             // system symbol ' ' is unquoted
                             << "opt2 = 1 =2"            // extra '='
                             << "opt3 = ;"               // missing param
                             << "opt4 = ,"               // system symbol is unquoted
                             << "method5 ( 1 1"          // no ending ')'
                             << "method6 1 1 )"          // system symbol is unquoted
                             << "method7(1 , ,1);"       // missed param
                             << "method8 ( 1 (1 1));"    // extra '('
                             << "method9 1 a, b, c 2"    // system symbol ' ' is unquoted
                             << "method10 \'=\'  "       // invalid name
                             << "method11 (=)"           // system symbol is unquoted
                             << "method12 a, b,"         // missing param
                             << "method13 a, b,\n \t\n"; // missing param
                                                       
        CheckTable tWrong( domain, rawWrong, rawWrong.size() );
        tWrong.Check();
    }

} // namespace

// ---------------------------------------

namespace 
{
    class TestParams {};
    
    class StreamParserTestsClass
    {
    public:
        StreamParserTestsClass(Domain::DomainClass &domain, TestParams &params)
        {
            SplitingDataAndOkNameParsingTest(domain);
            WrongNamesParsingTest(domain);
            OkParamsParsingTest(domain);
            WrongParamsParsingTest(domain);

            domain.Stop(Domain::DomainExitOk);
        }
    };
} // namespace

// ---------------------------------------
    
namespace
{
    void StreamParserTests(bool silentMode)
    {
        const Platform::dword CTestTimeoutMS = 10 * 1000;
        Domain::DomainTestRunner runner(CTestTimeoutMS);
        
        TestParams params;
        bool res = runner.Run<StreamParserTestsClass>(params);
        TUT_ASSERT(res);

        if (!silentMode) std::cout << "StreamParserTests: OK\n";
    }

    // --------------------------------------

    void DRIParserHelpersTest(bool silentMode)
    {
        // check return val; check skipEmpty flag; check splitting
        QString last = "data5";
        QStringList correctDataList = QStringList() << "" << "data1" << "data2" << "\"data3, data3, \"" 
                            << "data4" << last;
        QChar sep = ',';
        QString data = correctDataList.join(sep);
        QString dataEndsWithSep = data + ",";

        QStringList checkDataList;
        int lastSep = SplitWithQuotesConsideration(data, sep, checkDataList, false);
        TUT_ASSERT( lastSep == (data.size() - last.size() - 1) );
        TUT_ASSERT( checkDataList.size() == correctDataList.size() );
        TUT_ASSERT(checkDataList == correctDataList);

        correctDataList.removeFirst(); // remove empty to check
        lastSep = SplitWithQuotesConsideration(dataEndsWithSep, sep, checkDataList, true);
        TUT_ASSERT( lastSep == (dataEndsWithSep.size() - 1) );
        TUT_ASSERT( checkDataList.size() == correctDataList.size() );
        TUT_ASSERT(checkDataList == correctDataList);
       
        if (!silentMode) std::cout << "DRIParserHelpersTest: OK\n";
    }

} // namespace

// --------------------------------------

namespace DriTests
{
    void DriParserTests(bool silentMode)
    {
        StreamParserTests(silentMode);
        DRIParserHelpersTest(silentMode);
    }
    
} // namespace DriTests

