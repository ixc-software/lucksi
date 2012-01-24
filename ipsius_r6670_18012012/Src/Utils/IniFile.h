#ifndef __UTILSINIFILE__
#define __UTILSINIFILE__

#include "Utils/IBasicInterface.h"
#include "Utils/ManagedList.h"
#include "Utils/StringUtils.h"
#include "Utils/IntToString.h"

namespace Utils
{

    class IIniFileDataSource : public Utils::IBasicInterface
    {
    public:
        virtual bool Eof() = 0;
        virtual std::string ReadNextLine() = 0;
    };

    // --------------------------------------------

    class IniFileManagedListLoader : public IIniFileDataSource
    {
        const ManagedList<std::string> &m_src;
        int m_currLine;

    // IIniFileDataSource impl
    private:

        bool Eof()
        {
            return ( m_currLine >= m_src.Size() );
        }

        std::string ReadNextLine()
        {
            return *( m_src[m_currLine++] );
        }

    public:

        IniFileManagedListLoader(const ManagedList<std::string> &src) : 
          m_src(src), m_currLine(0) 
          {
          }

    };

    // --------------------------------------------

    struct IniFileConfig
    {
        std::string Comment;
        char        StringChar;
        bool        AllowSections;
        bool        AllowDublicateSections;
        bool        AllowDublicateItems;

        IniFileConfig()
        {
            Comment = "//";
            StringChar = '\"';
            AllowSections = false;
            AllowDublicateSections = false;
            AllowDublicateItems = false;
        }

    };

    // --------------------------------------------

    ESS_TYPEDEF(IniFileParseError);

    struct IniFileUtils
    {
        static void ThrowParseError(const std::string &msg, 
            int indx = -1, const std::string &data = "")
        {
            std::ostringstream oss;
            
            oss << msg;

            if (indx >= 0)
            {
                oss << " at " << indx << "; data: " << data;
            }

            ESS_THROW_MSG(IniFileParseError, oss.str());
        }

        static bool IdentificatorOK(const std::string &s)
        {
            for(int i = 0; i < s.size(); ++i)
            {
                char c = s.at(i);
                bool isAlpha = CharIsAlpha(c);
                bool isDigit = CharIsDigit(c);
                bool isSpec = CharIsSpecial(c);

                if (i == 0)
                {
                    if (!isAlpha) return false;
                    continue;
                }

                if (! (isAlpha || isDigit || isSpec)) return false;
            }

            return true;
        }

        static void VerifyIdentificator(const std::string &i,
            int indx = -1, const std::string &data = "")
        {
            if (!IdentificatorOK(i)) 
            {
                std::string s("Bad identificator ");
                s += i;
                ThrowParseError(s, indx, data);
            }
        }

        static std::string ItemSep()
        {
            static std::string s("=");
            return s;
        }

    private:

        static bool CharIsAlpha(char c)
        {
            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
        }

        static bool CharIsDigit(char c)
        {
            return (c >= '0' && c <= '9');
        }

        static bool CharIsSpecial(char c)
        {
            static std::string s("_-");
            return (s.find(c) != std::string::npos);
        }

    };

    // --------------------------------------------

    class IniFileItem : boost::noncopyable
    {
        std::string m_name;   // const
        std::string m_value;  // const 
        int m_lineIndex;

    public:

        IniFileItem(const std::string &line, int index, char stringChar) :
            m_lineIndex(index)
        {
            size_t pos = line.find( IniFileUtils::ItemSep() );

            m_name  = Utils::TrimBlanks( line.substr(0, pos) );
            if (pos != std::string::npos)
            {
                m_value = Utils::TrimBlanks( line.substr(pos + 1) );
            }

            // unquote m_value
            if (m_value.size() > 1)
            {
                if ( (m_value.at(0) == stringChar) && 
                     (m_value.at( m_value.size() - 1 ) == stringChar) )
                {
                    m_value = m_value.substr(1, m_value.size() - 2);
                }
            }
        }

        const std::string& Name() const { return m_name; }
        const std::string& Value() const { return m_value; }

        std::string AsString() const { return m_value; }

        bool AsBool(bool *pConvertErr = 0) const // can throw ConvertException, if pConvertErr = 0
        {
            if (pConvertErr != 0) *pConvertErr = false;

            if (m_value == "0") return false;
            if (m_value == "1") return true;

            std::string s = Utils::StringUpCase(m_value);

            if (s == "FALSE" || s == "F") return false;
            if (s == "TRUE"  || s == "T") return true;

            // error
            if (pConvertErr != 0)  *pConvertErr = true;
            else                   ThrowConvertException("bool");
            
            // dummi
            return false;
        }

        int AsInt(bool *pConvertErr = 0) const  /* - // - */
        {
            int val;
            if (Utils::StringToInt(m_value, val))
            {
                if (pConvertErr != 0) *pConvertErr = false;
                return val;
            }

            // error
            if (pConvertErr != 0)  *pConvertErr = true;
            else                   ThrowConvertException("int");

            // dummi
            return -1;
        }

        ESS_TYPEDEF(ConvertException);

    private:

        void ThrowConvertException(const std::string &typeName) const 
        {
            std::ostringstream oss;

            oss << "Can't convert item's " << m_name << " value " << m_value 
                << " to " << typeName << "in line " << m_lineIndex;

            ESS_THROW_MSG(ConvertException, oss.str());
        }

    };

    // --------------------------------------------

    class IniFileSection : boost::noncopyable
    {
        const std::string m_name;
        Utils::ManagedList<IniFileItem> m_items;

    public:

        ESS_TYPEDEF(ItemNotFound);

        IniFileSection(const std::string &name) : 
          m_name(name)
        {
            IniFileUtils::VerifyIdentificator(m_name);
        }

        void AddItem(IniFileItem *pItem, bool dublicatesCheck) // can throw IniFileParseError 
        {
            ESS_ASSERT(pItem != 0);

            if (dublicatesCheck)
            {
                if (FindItem(pItem->Name()) != 0) 
                {
                    std::string msg = "Dublicate item ";
                    msg += pItem->Name();
                    IniFileUtils::ThrowParseError(msg);
                }
            }

            m_items.Add(pItem);
        }

        const std::string& Name() const { return m_name; }

        int ItemsCount() const { return m_items.Size(); }
        const IniFileItem& operator[] (int indx) const { return *(m_items[indx]); }

        const IniFileItem* FindItem(const std::string &name) const 
        {
            for(int i = 0; i < m_items.Size(); ++i)
            {
                if (m_items[i]->Name() == name) return m_items[i];
            }

            return 0;
        }

        const IniFileItem& Item(const std::string &name) const // can throw if item not found 
        {
            const IniFileItem *pI = FindItem(name);
            if (pI == 0) ThrowItemNotFound(name);

            return *pI;
        }

        std::string GetItemValue(const std::string &name) const // can throw if item not found 
        {
            const IniFileItem *pI = FindItem(name);
            if (pI == 0) ThrowItemNotFound(name);

            return pI->Value();
        }

    private:

        void ThrowItemNotFound(const std::string &name) const
        {
            std::string msg("Item not found, name ");
            msg += name;
            ESS_THROW_MSG(ItemNotFound, msg);
        }

    };

    // --------------------------------------------
    
    class IniFile : boost::noncopyable
    {

        template<class TCont, class TConvFunc>
        class ContDataSrc : public IIniFileDataSource
        {
            typedef typename TCont::const_iterator iter;

            const TConvFunc &m_convFunc;
            iter m_curr;
            iter m_end;

        // IIniFileDataSource impl
        private:

            bool Eof()
            {
                return (m_curr == m_end);
            }

            std::string ReadNextLine()
            {
                ESS_ASSERT(!Eof());

                return m_convFunc(*m_curr++);
            }

        public:

            ContDataSrc(const TCont &cont, const TConvFunc &convFunc) : 
              m_convFunc(convFunc), m_curr(cont.begin()), m_end(cont.end())
            {
            }
        };

        class ConvertFunctor
        {
        public:

            std::string operator() (const std::string &s) const
            {
                return s;
            }

        };

        Utils::ManagedList<IniFileSection> m_sections;

        static std::string SimplifyLine(const std::string &sIn, const IniFileConfig &cfg)
        {
            std::string s = sIn;
            s = TrimBlanks(s);

            int inString = 0;

            for(int i = 0; i < s.size(); ++i)
            {
                if (s.at(i) == cfg.StringChar) inString++;

                if ((inString & 1) == 0)  // outside string
                {
                    if (s.find_first_of(cfg.Comment, i) == i)
                    {
                        return TrimBlanks(s.substr(0, i));
                    }
                }
            }

            return s;
        }

        IniFileSection* CreateAndRegisterSection(const std::string &name, bool dublicatesCheck)
        {
            // dublicates check
            if (dublicatesCheck)
            {
                if (SectionByName(name) != 0) 
                {
                    std::string s = "Dublicate section, name ";
                    s += name;
                    IniFileUtils::ThrowParseError(s);
                }
            }

            IniFileSection *p = new IniFileSection(name);

            m_sections.Add(p);

            return p;
        }

        void Load(IIniFileDataSource &src, const IniFileConfig &cfg)
        {
            IniFileSection *pCurrSection = 0;

            int counter = -1;

            while(!src.Eof())
            {
                std::string line = src.ReadNextLine();
                counter++;

                // cut comments
                std::string s = SimplifyLine(line, cfg);
                if (s.empty()) continue;

                // item vs section 
                if (s.at(0) == '[')
                {
                    if (!cfg.AllowSections)
                    {
                        IniFileUtils::ThrowParseError("Sections not allowed", counter, line);
                    }

                    if ( s.at(s.size() - 1) == ']') 
                    {
                        pCurrSection = 
                            CreateAndRegisterSection(s.substr(1, s.size() - 2),
                                                     !cfg.AllowDublicateSections);
                        continue;
                    }

                    IniFileUtils::ThrowParseError("Bad section syntax", counter, line);
                }
                else
                {
                    if (pCurrSection == 0) 
                    {
                        pCurrSection = CreateAndRegisterSection("default", true);
                    }
                    pCurrSection->AddItem( new IniFileItem(s, counter, cfg.StringChar), !cfg.AllowDublicateItems );
                }
            }
        }


        template<class TConvFunc, class TCont>
        void Init(const TCont &src, const TConvFunc &fn, const IniFileConfig &cfg)
        {
            ContDataSrc<TCont, TConvFunc> contSrc(src, fn);
            Load(contSrc, cfg);
        }

    protected:

        IniFile(IIniFileDataSource &src, const IniFileConfig &cfg)
        {
            Load(src, cfg);
        } 

        template<class TConvFunc, class TCont>
        IniFile(const TCont &src, const TConvFunc &fn, const IniFileConfig &cfg)
        {
            Init(src, fn, cfg);
        }

    public:
                        
        int SectionsCount() const
        {
            return m_sections.Size();
        }

        const IniFileSection& operator[] (int indx)
        {
            return *(m_sections[indx]);
        }

        const IniFileSection* SectionByName(const std::string &name)
        {
            for(int i = 0; i < m_sections.Size(); ++i)
            {
                if (m_sections[i]->Name() == name) return m_sections[i];
            }

            return 0;
        }

        template<class TCont>  // TCont = any container<std::string>
        static IniFile* LoadFromStringContainer( const TCont &src, 
            const IniFileConfig &cfg = IniFileConfig() )
        {
            ConvertFunctor fn;
            return new IniFile(src, fn, cfg);
        }

        static IniFile* LoadFromManagedList( const ManagedList<std::string> &src,
            const IniFileConfig &cfg = IniFileConfig() )
        {
            IniFileManagedListLoader loader(src);
            return new IniFile( loader, cfg );
        }

    };
    
}  // namespace Utils

#endif
