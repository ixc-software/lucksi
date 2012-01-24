#include "stdafx.h"
#include "MetaConfig.h"
#include "StringList.h"
#include "QtHelpers.h"
#include "ExeName.h"


// -----------------------------------------------------------

namespace
{
    const QString CNewLineSep = "\\";

} // namespace 

// -----------------------------------------------------------

namespace
{
    using namespace Utils;

    class TestBasic
    {
    public:
        QString TestName;
        QString LogFileName;
        int TimeToRunMs;

        TestBasic() : TimeToRunMs(0)
        {
            // ...
        }

        virtual ~TestBasic() 
        {
        }

        void LoadFrom(StringList sl)
        {
            UpdateMeta();
            MetaConfigLoader loader(sl);
            m_config.LoadFrom(loader);
        }

        void SaveTo(StringList &sl)
        {
            UpdateMeta();
            m_config.SaveTo(sl);
        }

        QString GetInfo() const
        {
            return m_config.ListAllFields();
        }

    protected:

        virtual void AddFields(MetaConfig &config)
        {
            config.Add(TestName, "TestName", "Test name, formal");
            config.Add(TimeToRunMs, "TimeToRunMs", "Time to run in ms");
            config.Add(LogFileName, "LogFileName");
        }

    private:

        void UpdateMeta()
        {
            if (m_config.getCompleted()) return;

            AddFields(m_config);
            m_config.Complete();
        }

        MetaConfig m_config;
    };

    class TestLapd : public TestBasic
    {
        void AddFields(MetaConfig &config)  // override
        {
            TestBasic::AddFields(config);
            config.Add(DropPackets, "DropPackets", "Enable drop L2 packets");
        }

    public:
        bool DropPackets;

        TestLapd() : DropPackets(false)
        {

        }

    };

    class TestDss : public TestBasic
    {
        void AddFields(MetaConfig &config)  // override
        {
            TestBasic::AddFields(config);
            config.Add(EnableLoging, "EnableLoging", "Enable loging");
        }

    public:
        bool EnableLoging;

        TestDss() : EnableLoging(false)
        {

        }

    };

    void PrintStringList(const StringList &sl)
    {
        for(int i = 0; i < sl.size(); ++i)
        {
            QString s = sl.at(i);
            std::cout << s;
        }
    }

    void ExecTest(bool silentMode)
    {
        using std::endl;

        StringList sl;

        sl 
            << "TestName = TestLapd  // comment..."
            << "TimeToRunMs = 100"
            << "DropPackets = true"
            << "LogFileNAME =";   // empty value, case ignore

        // load
        MetaConfigLoader loader(sl);
        TUT_ASSERT(loader.NameAt(0) == "TestName");

        QString testType = loader.ValueAt(0);

        if (testType == "TestLapd")
        {
            TestLapd test;
            test.LoadFrom(sl);

            TUT_ASSERT( test.TestName == "TestLapd");
            TUT_ASSERT( test.DropPackets );

            if (!silentMode) std::cout << test.GetInfo() << endl << endl;

            StringList sl;
            test.SaveTo(sl);
            if (!silentMode) PrintStringList(sl);

            return;
        }

        if (testType == "TestDss")
        {
            // ...
        }

        TUT_ASSERT(0 && "Bad test type");
    }

    // ------------------------------------------------------------

    class SyntaxTestClass
    {
        MetaConfig m_config;

        // registered fields
        bool m_optBool;
        int m_optInt;
        QString m_optQString;

        enum ConfigStates
        {
            NoErrSt,
            SyntaxErrSt,
            DublicateNameSt,
            BadConversionSt,
            UnknownFieldNameSt,
        };

        ConfigStates ConfigOk(const StringList &confList)
        {
            ConfigStates res = NoErrSt;

            try
            {
                MetaConfigLoader loader(confList);
                m_config.LoadFrom(loader);
            }
            catch(MetaConfigLoader::SyntaxError) { res = SyntaxErrSt; }
            catch(MetaConfigLoader::DublicateName) { res = DublicateNameSt; }
            catch(MetaConfig::BadConversion) { res = BadConversionSt; }
            catch(MetaConfig::UnknownFieldName) { res = UnknownFieldNameSt; }

            return res;
        }
        
        void RegisterMeta()
        {
            m_config.Add(m_optBool, "optBool", "");
            m_config.Add(m_optInt, "optInt", "");
            m_config.Add(m_optQString, "optQString", "");
            m_config.Complete();
        }

        static QString AddSep(const QString &str)
        {
            return QString(str + CNewLineSep);
        }

    public:
        SyntaxTestClass() : m_optBool(false), m_optInt(0)
        {
            RegisterMeta();

            StringList configOk1;
            configOk1 << AddSep("optBool = t") << "rue" << "optInt = 1";
            TUT_ASSERT( ConfigOk(configOk1) == NoErrSt);

            // check cascade-style option writing; check cutting comment
            StringList configOk2;
            configOk2 << AddSep("optQString = begin ") << QString(CNewLineSep + " //comment") 
                        << AddSep("text ") << "end";
            TUT_ASSERT( ConfigOk(configOk2) == NoErrSt);
            
            // check that separator is ok between two options
            StringList configOk3;
            configOk3 << AddSep("optBool = 1 ") << "optQString = text";
            TUT_ASSERT( ConfigOk(configOk3) == NoErrSt);

            // check that separator is ok after '='
            StringList configOk4;
            configOk4 << AddSep("optBool = ") << AddSep("true") << AddSep("optInt = 100") 
                        << AddSep("optQString = text");
            TUT_ASSERT( ConfigOk(configOk4) == NoErrSt);
            TUT_ASSERT(m_optBool);
            TUT_ASSERT(m_optInt == 100);
            TUT_ASSERT(m_optQString == "text");

            // check right-parsing for quoted text
            StringList configOk5;
            configOk5 << AddSep("optQString = \"begin \t\"") << AddSep("\"text \"") << "end";
            TUT_ASSERT( ConfigOk(configOk5) == NoErrSt);
            
            // check line without separator is ok if only previous line has one
            StringList configWr1;
            configWr1 << "optQString = begin " << CNewLineSep << AddSep("text ") << "end";
            TUT_ASSERT( ConfigOk(configWr1) == SyntaxErrSt);
            
            // check syntacsis: 'option = value'
            StringList configWr2;
            configWr2 << AddSep("optQString = some ") << "text " << "optBool";
            TUT_ASSERT( ConfigOk(configWr2) == SyntaxErrSt);

            // check DublicateName errror
            StringList configWr3;
            configWr3 << "optQString = text" << "optQString = text2" << "optBool = T";
            TUT_ASSERT( ConfigOk(configWr3) == DublicateNameSt);

            // check BadConversion error
            StringList configWr4;
            configWr4 << "optInt = text";
            TUT_ASSERT( ConfigOk(configWr4) == BadConversionSt);

            // check UnknownFieldName error
            StringList configWr5;
            configWr5 << "optQString = text" << "_unknownOpt_ = text2";
            TUT_ASSERT( ConfigOk(configWr5) == UnknownFieldNameSt);
        }

    };

    // -----------------------------------------------------------

    void SyntaxTest()
    {
        SyntaxTestClass test;
    }

} // namespase

// -----------------------------------------------------------

namespace Utils
{
    /*
    // original Load(); does not handle '\'
    void MetaConfigLoader::Load(StringList sl, bool namesIgnoreCase)
    {
        for (int i = 0; i < sl.size(); ++i) 
        {
            QString s = sl.at(i);
            QString errInfo = "Line " + QString::number(i + 1) + ": " + s;

            s = s.trimmed();
            if (s.isEmpty()) continue;
            std::cout << "s = " << s.toStdString() << std::endl;
            // cut comment
            int pos = s.indexOf("//");
            if (pos >= 0)
            {
                s = s.left(pos).trimmed();
                if (s.size() == 0) continue;
            }

            // name = val
            pos = s.indexOf('=');
            if (pos < 0) ESS_THROW_MSG(SyntaxError, errInfo.toStdString());

            Pair pair;
            pair.Name = s.left(pos).trimmed();
            pair.Value = s.mid(pos + 1).trimmed();

            if (NameExists(pair.Name, namesIgnoreCase)) 
            {
                ESS_THROW_MSG(DublicateName, errInfo.toStdString());
            }
            m_pairs.push_back(pair);
        }
    }
    */
    
    // -----------------------------------------------------------------
   
    namespace
    {
        void CutCommentAndTrim(QString &str)
        {
            str = str.trimmed();
            if (str.isEmpty()) return;
            
            // cut comment
            int pos = str.indexOf("//");
            if (pos >= 0) str = str.left(pos).trimmed();
        }

        // -----------------------------------------------------------------

        void RemoveNewLineSepAndTrim(QString &str, bool &nextStrIsValue)
        {
            if ( !str.endsWith(CNewLineSep) )
            {
                nextStrIsValue = false;
                return;
            }

            int pos = str.indexOf(CNewLineSep);
            str = str.left(pos).trimmed();
            nextStrIsValue = true;
        }

    } // namespace

    // -----------------------------------------------------------------

    void MetaConfigLoader::Load(StringList sl, bool namesIgnoreCase)
    {
        Pair pair;
        bool nextStrIsValue = false;

        for (int i = 0; i < sl.size(); ++i) 
        {
            QString s = sl.at(i);
            QString errInfo = "Line " + QString::number(i + 1) + ": " + s;

            CutCommentAndTrim(s);
            if (s.isEmpty()) continue;
            
            int pos = s.indexOf('=');
            if (pos >= 0)
            {
                // add previuos pair
                if (nextStrIsValue) m_pairs.push_back(pair);

                // make next
                RemoveNewLineSepAndTrim(s, nextStrIsValue);

                //std::cout << "s = |" << s.toStdString() << "|" << std::endl;

                pair.Name = s.left(pos).trimmed();
                if ( NameExists(pair.Name, namesIgnoreCase) ) 
                {
                    ESS_THROW_MSG(DublicateName, errInfo.toStdString());
                }

                pair.Value = s.mid(pos + 1).trimmed();
            }
            else 
            {
                // only 'name'
                if (!nextStrIsValue) ESS_THROW_MSG(SyntaxError, errInfo.toStdString());

                RemoveNewLineSepAndTrim(s, nextStrIsValue);
                pair.Value += s;
            }

            if (nextStrIsValue) continue;

            //std::cout << "pair.Value = |" << s.toStdString() << "|" << std::endl;
            // add
            m_pairs.push_back(pair);
        }

        // add last one
        if (nextStrIsValue) m_pairs.push_back(pair);
    }
    
    // -----------------------------------------------------------------

    void MetaConfig::LoadFrom(const MetaConfigLoader &loader)
    {
        ESS_ASSERT(m_completed);

        for(int i = 0; i < loader.getCount(); ++i)
        {
            QString name = loader.NameAt(i);
            IConfigField *pF = FindByName(name, m_namesIgnoreCase);
            if (pF == 0) 
			{
				ESS_THROW_MSG(UnknownFieldName, name.toStdString());
				return;  // dummi 				
			}

            pF->set(loader.ValueAt(i));
        }
    }

    // -----------------------------------------------------------------

    void MetaConfig::SaveTo(StringList &sl)const
    {
        ESS_ASSERT(m_completed);

        for(int i = 0; i < m_fields.size(); ++i)
        {
            QString s = m_fields.at(i)->getInSaveFormat();
            sl << s;
        }

    }

    // -----------------------------------------------------------------

    void MetaConfig::RunTest(bool silentMode)
    {
        ExecTest(silentMode);
        SyntaxTest();
    }
	
}  // namespace Utils

