
#include "stdafx.h"
#include "IniFileTest.h"
#include "Utils/IniFile.h"


namespace
{
    bool IsExpectedError(std::string err, std::string text)
    {
        err =  Utils::StringUpCase(err);
        text = Utils::StringUpCase(text);
        
        size_t found = err.find(text, 0);

        return (found != std::string::npos);
    }
    
} // namespace

// ------------------------------------------------------------

namespace IniFileItemTest
{
    /* IniFileItem:

        exc: ConvertException
        
        IniFileItem(const std::string &line, int index, char stringChar);
        const std::string& Name() const;
        const std::string& Value() const;
        std::string AsString() const;
        bool AsBool(bool *pConvertErr = 0) const; // can throw ConvertException, if pConvertErr = 0
        int AsInt(bool *pConvertErr = 0) const;
    */

    using namespace Utils;

    struct ToParse
    {
        std::string Line;
        int StartIndex;
        char StringChar;

        ToParse(const std::string &line, int index, char stringChar) :
            Line(line), StartIndex(index), StringChar(stringChar)
        {
        }
    };

    class ToCheck
    {
        std::string m_name;
        std::string m_val;
        boost::shared_ptr<bool> m_valBool;
        boost::shared_ptr<int> m_valInt;

        bool m_silentMode;

    public:
        ToCheck(const std::string &name, 
                const std::string &val, 
                bool silentMode) :
            m_name(name), m_val(val), m_silentMode(silentMode)
        {
        }

        ToCheck(const std::string &name, 
                const std::string &val, 
                bool valBool,
                bool silentMode) :
            m_name(name), m_val(val), m_valBool(new bool(valBool)), m_silentMode(silentMode)
        {
        }

        ToCheck(const std::string &name, 
                const std::string &val, 
                int valInt,
                bool silentMode) :
            m_name(name), m_val(val), m_valInt(new int(valInt)), m_silentMode(silentMode)
        {
        }

        bool IsBoolVal() const { return (m_valBool != 0); }
        bool IsIntVal() const { return (m_valInt != 0); }

        void Check(const IniFileItem &item) const
        {
            if (!m_silentMode) 
            {
                std::cout << "__________________" << std::endl
                    << "Name: " << m_name << " / " << item.Name() << std::endl
                    << "Val: " << m_val << " / " << item.Value() << std::endl
                    << "AsStr: " << m_val << " / " << item.AsString() << std::endl;
            }
            
            TUT_ASSERT(m_name == item.Name());
            TUT_ASSERT(m_val == item.Value());
            TUT_ASSERT(m_val == item.AsString());
        }

        void CheckBool(const IniFileItem &item, bool withException) const
        {
            TUT_ASSERT(IsBoolVal());

            try
            {
                if (!m_silentMode) 
                {
                    std::cout << "AsBool: " << *m_valBool.get() 
                        << " / " << item.AsBool() << std::endl;
                }
                
                TUT_ASSERT(*m_valBool.get() == item.AsBool());
            }
            catch (IniFileItem::ConvertException &e)
            {
                TUT_ASSERT(withException);
                return;
            }

            TUT_ASSERT(!withException);
        }

        void CheckInt(const IniFileItem &item, bool withException) const
        {
            TUT_ASSERT(IsIntVal());
            
            try
            {
                if (!m_silentMode) 
                {
                    std::cout << "AsInt: " << *m_valInt.get() 
                    << " / " << item.AsInt() << std::endl;
                }
                
                TUT_ASSERT(*m_valInt.get() == item.AsInt());
            }
            catch (IniFileItem::ConvertException &e)
            {
                TUT_ASSERT(withException);
                return;
            }

            TUT_ASSERT(!withException);
        }
    };

    // ---------------------------------------------------------

    void Check(const ToParse &item, const ToCheck &toCheck, bool withException = false)
    {
        IniFileItem parsed(item.Line, item.StartIndex, item.StringChar);

        toCheck.Check(parsed);
        
        if (toCheck.IsBoolVal()) toCheck.CheckBool(parsed, withException);
        if (toCheck.IsIntVal()) toCheck.CheckInt(parsed, withException);
    }

    // ------------------------------------------------------------
    
    void Run(bool silentMode)
    {
        const bool withException = true;
        const char strCh = '\"';
        const int index = 0; // line # - using in exceptions

        // string
        Check(ToParse("name = val", index, strCh),          ToCheck("name", "val", silentMode));
        Check(ToParse("name1 = \"val1\"", index, strCh),    ToCheck("name1", "val1", silentMode));
        Check(ToParse("name1 = \"val\"1", index, strCh),    ToCheck("name1", "\"val\"1", silentMode));
        Check(ToParse("1 = 1", index, strCh),               ToCheck("1", "1", silentMode));
        Check(ToParse("1name = 1val", index, strCh),        ToCheck("1name", "1val", silentMode));
        Check(ToParse("name1=val1", index, strCh),          ToCheck("name1", "val1", silentMode));
        Check(ToParse("name1 = val1 = val2", index, strCh), ToCheck("name1", "val1 = val2", silentMode));
        Check(ToParse("name1 = ", index, strCh),            ToCheck("name1", "", silentMode));
        Check(ToParse("long name 1 = val 1", index, strCh), ToCheck("long name 1", "val 1", silentMode));
        Check(ToParse("n1 == v1", index, strCh),            ToCheck("n1", "= v1", silentMode));
        Check(ToParse("!@#$%^&*()_+ = /*-:\"|{}[]", index, strCh), 
              ToCheck("!@#$%^&*()_+", "/*-:\"|{}[]", silentMode));

        Check(ToParse("name1", index, strCh),    ToCheck("name1", "", silentMode),     withException);
        Check(ToParse("= val1", index, strCh),   ToCheck("", "val1", silentMode),      withException);
        Check(ToParse("=", index, strCh),        ToCheck("", "", silentMode),          withException);
        Check(ToParse("  =  ", index, strCh),    ToCheck("", "", silentMode),          withException);
        
        // bool
        Check(ToParse("name1 = true", index, strCh),     ToCheck("name1", "true", true, silentMode)); 
        Check(ToParse("name1 = \"true\"", index, strCh), ToCheck("name1", "true", true, silentMode)); 
        Check(ToParse("name1 = TRUE", index, strCh),     ToCheck("name1", "TRUE", true, silentMode)); 
        Check(ToParse("name1 = tRue", index, strCh),     ToCheck("name1", "tRue", true, silentMode)); 
        Check(ToParse("name1 = T", index, strCh),        ToCheck("name1", "T", true, silentMode)); 
        Check(ToParse("name1 = 1", index, strCh),        ToCheck("name1", "1", true, silentMode)); 
        Check(ToParse("name1 = false", index, strCh),    ToCheck("name1", "false", false, silentMode)); 
        Check(ToParse("name1 = FALSE", index, strCh),    ToCheck("name1", "FALSE", false, silentMode)); 
        Check(ToParse("name1 = F", index, strCh),        ToCheck("name1", "F", false, silentMode)); 
        Check(ToParse("name1 = 0", index, strCh),        ToCheck("name1", "0", false, silentMode)); 

        Check(ToParse("name1 = invalid_bool", index, strCh), 
              ToCheck("name1", "invalid_bool", false, silentMode), 
              withException); 
        
        // int
        Check(ToParse("name1 = 10", index, strCh),     ToCheck("name1", "10", 10, silentMode)); 
        Check(ToParse("name1 = \"10\"", index, strCh), ToCheck("name1", "10", 10, silentMode)); 
        Check(ToParse("name1 = -10", index, strCh),    ToCheck("name1", "-10", -10, silentMode));

        Check(ToParse("name1 = invalid_int", index, strCh),    
              ToCheck("name1", "invalid_int", 0, silentMode), 
              withException);
    }

    // ------------------------------------------------------------
    
} // IniFileItemTest

// ------------------------------------------------------------

namespace IniFileSectionTest
{
    /* IniFileSection:

        exc: ItemNotFound

        IniFileSection(const std::string &name);
        void AddItem(IniFileItem *pItem, bool dublicatesCheck);
        const std::string& Name() const;
        int ItemsCount() const;
        const IniFileItem& operator[] (int indx) const;

        const IniFileItem* FindItem(const std::string &name) const ;
        const IniFileItem& Item(const std::string &name) const;

        std::string GetItemValue(const std::string &name) const; // can throw if item not found 
    */

    using namespace Utils;
    
    void Run()
    {
        const char strCh = '\"';
        
        // check empty
        {
            IniFileSection s("section");
            TUT_ASSERT(s.Name() == "section");
            TUT_ASSERT(s.ItemsCount() == 0);
            TUT_ASSERT(s.FindItem("dummi") == 0);

            bool wasException = false;
            try
            {
                s.GetItemValue("dummi");
                
            }
            catch(IniFileSection::ItemNotFound &e)
            {
                wasException = true;
            }
            TUT_ASSERT(wasException);
        }
        // allow empty name
        {
            IniFileSection s("");
            TUT_ASSERT(s.Name() == "");
        }
        // check with !dublicate
        {
            bool dublicatesCheck = true;

            IniFileSection s("section");

            // p0, p1 will be deleted by IniFileSection
            IniFileItem* p0 = new IniFileItem("name0 = val0", 0, strCh);
            IniFileItem* p1 = new IniFileItem("name1 = val1", 1, strCh);
            
            s.AddItem(p0, dublicatesCheck);
            s.AddItem(p1, dublicatesCheck);
            TUT_ASSERT(s.ItemsCount() == 2);
            TUT_ASSERT(&s[0] == p0);
            TUT_ASSERT(&s[1] == p1);
            TUT_ASSERT(&s.Item("name0") == p0);
            TUT_ASSERT(s.FindItem("name0") == p0);
            TUT_ASSERT(s.GetItemValue("name0") == "val0");

            bool wasException = false;
            try
            {
                s.AddItem(new IniFileItem("name1 = val1", 0, strCh), dublicatesCheck);
            }
            catch (IniFileParseError &e)
            {
                TUT_ASSERT(IsExpectedError(e.getTextMessage(), "dublicate item"));
                wasException = true;
            }
            TUT_ASSERT(wasException);
        }
        // check with dublicate
        {
            bool dublicatesCheck = false;

            IniFileSection s("section");

            // p0 will be deleted by IniFileSection
            IniFileItem* p0 = new IniFileItem("name0 = val0", 0, strCh);
            IniFileItem* p1 = new IniFileItem("name0 = val1", 0, strCh);
            
            s.AddItem(p0, dublicatesCheck);
            s.AddItem(p1, dublicatesCheck); // ok
        }
    }
    
} // IniFileSectionTest

// ------------------------------------------------------------

namespace IniFileTest
{
    /* IniFile:

        int SectionsCount() const;
        const IniFileSection& operator[] (int indx);
        
        const IniFileSection* SectionByName(const std::string &name);

        // TCont = any container<std::string>
        template<class TCont> 
        static IniFile* LoadFromStringContainer(const TCont &src, 
                                                const IniFileConfig &cfg = IniFileConfig());
    */
    /* 
    struct IniFileConfig
    {
        std::string Comment;
        char        StringChar;
        bool        AllowSections;
        bool        AllowDublicateSections;
        bool        AllowDublicateItems;
    };
    */

    using namespace Utils;

    std::string MakeComment(const std::string &comment, const std::string &data)
    {
        std::string res(comment);
        res += " ";
        res += data;

        return res;
    }

    // ------------------------------------------------------------

    std::string LineWithCommnet(const std::string &data, 
                                const std::string &comment,
                                const std::string &commentText)
    {
        std::string res(data);
        res += " ";
        res += MakeComment(comment, commentText);

        return res;
    }

    // ------------------------------------------------------------

    std::string LineWithWrapValue(char strChar, 
                                  const std::string &var,
                                  const std::string &value)
    {
        std::string res(var);
        res += " = ";
        res += strChar;
        res += value;
        res += strChar;

        return res;
    }
    
    // ------------------------------------------------------------
    
    void MakeIniFile_AllowAll(std::vector<std::string> &iniFile, 
                              const IniFileConfig &config)
    {
        iniFile.clear();
        char ch = config.StringChar;
        std::string cmt(config.Comment); 

        iniFile.push_back(MakeComment(cmt, "IniFile"));

        iniFile.push_back("[section_1]");
        iniFile.push_back("name11 = value11");
        iniFile.push_back(LineWithCommnet("name12 = value12", cmt, "comment"));
        iniFile.push_back(LineWithWrapValue(ch, "name13", "value13"));
        iniFile.push_back("name14 = 4");
        iniFile.push_back("name15 = true");
        
        iniFile.push_back(LineWithCommnet("[section_2]", cmt, "description"));
        // dublicate item names inside one section
        iniFile.push_back("name21 = value21");
        iniFile.push_back("name21 = value21");
        
        iniFile.push_back(LineWithCommnet("[section_1]", cmt, "continue section_1"));
        iniFile.push_back(LineWithWrapValue(ch, "name16", "value 16"));
        
        iniFile.push_back("   [section_3]   ");
        // dublicate item names in diff section
        iniFile.push_back("name11 = value11");
        iniFile.push_back("name31 = 31");
    }

    // ------------------------------------------------------------

    void MakeIniFile_NotAllowSections(std::vector<std::string> &iniFile, 
                                      const IniFileConfig &config,
                                      bool withException)
    {
        iniFile.clear();
        char ch = config.StringChar;
        std::string cmt(config.Comment); 

        iniFile.push_back(MakeComment(cmt, "IniFile"));

        if (withException) iniFile.push_back("[section_dummi]");

        iniFile.push_back("name11 = value11");
    }

    // ------------------------------------------------------------
    
    void MakeIniFile_NotAllowDublicateSection(std::vector<std::string> &iniFile, 
                                              const IniFileConfig &config)
    {
        iniFile.clear();
        char ch = config.StringChar;
        std::string cmt(config.Comment); 

        iniFile.push_back("[section_1]");
        iniFile.push_back("name1 = value1");

        iniFile.push_back("[section_2]");
        iniFile.push_back("name2 = value2");
        
        iniFile.push_back("[section_1]");
    }

    // ------------------------------------------------------------

    void MakeIniFile_NotAllowDublicateItems(std::vector<std::string> &iniFile, 
                                            const IniFileConfig &config,
                                            bool withException)
    {
        iniFile.clear();
        char ch = config.StringChar;
        std::string cmt(config.Comment); 

        iniFile.push_back("[section_1]");
        iniFile.push_back("name1 = value1");
        
        iniFile.push_back("[section_2]");
        iniFile.push_back("name1 = value1"); // ok in different sections
        iniFile.push_back("name2 = value2");
        if (withException) iniFile.push_back("name2 = value2"); // error
    }
    
    // ------------------------------------------------------------
    
    void Run()
    {
        IniFileConfig config;
        config.Comment = "/*";
        config.StringChar = '\'';

        {   
            config.AllowSections = true;
            config.AllowDublicateSections = true;
            config.AllowDublicateItems = true;
            
            std::vector<std::string> f;
            MakeIniFile_AllowAll(f, config);
    
            boost::scoped_ptr<IniFile> p(IniFile::LoadFromStringContainer(f, config));

            TUT_ASSERT(p->SectionsCount() == 4);

            TUT_ASSERT((*p)[0].Name() == "section_1");
            // it doesn't store items of 2 sections with the same name in one
            TUT_ASSERT((*p)[0].ItemsCount() == 5); 

            TUT_ASSERT((*p)[1].Name() == "section_2");

            TUT_ASSERT(p->SectionByName("section_3") != 0);
            TUT_ASSERT(p->SectionByName("section_3")->Name() == "section_3");
            TUT_ASSERT(p->SectionByName("unknown_section") == 0);
        }
        {
            config.AllowSections = false;
            config.AllowDublicateSections = true;
            config.AllowDublicateItems = true;
            bool withException = false;

            std::vector<std::string> f;
            MakeIniFile_NotAllowSections(f, config, withException);

            boost::scoped_ptr<IniFile> p(IniFile::LoadFromStringContainer(f, config));

            TUT_ASSERT(p->SectionsCount() == 1);
            TUT_ASSERT((*p)[0].Name() == "default");
            TUT_ASSERT((*p)[0].ItemsCount() == 1);
        }
        {
            config.AllowSections = false;
            config.AllowDublicateSections = true;
            config.AllowDublicateItems = true;
            bool withException = true;

            std::vector<std::string> f;
            MakeIniFile_NotAllowSections(f, config, withException);

            bool wasException = false;
            try
            {
                boost::scoped_ptr<IniFile> p(IniFile::LoadFromStringContainer(f, config));
            }
            catch (IniFileParseError &e)
            {
                TUT_ASSERT(IsExpectedError(e.getTextMessage(), "sections not allowed"));
                wasException = true;
            }
            TUT_ASSERT(wasException);
        }
        {
            config.AllowSections = true;
            config.AllowDublicateSections = false;
            config.AllowDublicateItems = true;

            std::vector<std::string> f;
            MakeIniFile_NotAllowDublicateSection(f, config);

            bool wasException = false;
            try
            {
                boost::scoped_ptr<IniFile> p(IniFile::LoadFromStringContainer(f, config));
            }
            catch (IniFileParseError &e)
            {
                TUT_ASSERT(IsExpectedError(e.getTextMessage(), "dublicate section"));
                wasException = true;
            }
            TUT_ASSERT(wasException);
        }
        {
            config.AllowSections = true;
            config.AllowDublicateSections = true;
            config.AllowDublicateItems = false;
            bool withException = false;

            std::vector<std::string> f;
            MakeIniFile_NotAllowDublicateItems(f, config, withException);
            
            boost::scoped_ptr<IniFile> p(IniFile::LoadFromStringContainer(f, config));
            TUT_ASSERT(p->SectionByName("section_2")->ItemsCount() == 2);
        }
        {
            config.AllowSections = true;
            config.AllowDublicateSections = true;
            config.AllowDublicateItems = false;
            bool withException = true;

            std::vector<std::string> f;
            MakeIniFile_NotAllowDublicateItems(f, config, withException);
            
            bool wasException = false;
            try
            {
                boost::scoped_ptr<IniFile> p(IniFile::LoadFromStringContainer(f, config));
            }
            catch (IniFileParseError &e)
            {
                TUT_ASSERT(IsExpectedError(e.getTextMessage(), "dublicate item"));
                wasException = true;
            }
            TUT_ASSERT(wasException);
        }
    }
    
} // namespace IniFileTest

// ------------------------------------------------------------

namespace UtilsTests
{
    void IniFileTest(bool silentMode)
    {
        IniFileItemTest::Run(silentMode);
        IniFileSectionTest::Run();
        IniFileTest::Run();

        if (!silentMode) std::cout << "IniFile test: OK" << std::endl;
    }

} // namespace UtilsTests
