
#ifndef __DRIDEFINEANDMACROLIST__
#define __DRIDEFINEANDMACROLIST__

// DriDefineAndMacroList.h

#include "DRIParserHelpers.h"

#include "Utils/QtHelpers.h"
#include "Utils/ErrorsSubsystem.h"

namespace DRI
{
    // Save defines and macroses in one list.
    // 1) #define name value
    //    CS_Exec "file" "name = value"
    // using: %name%
    // 2) command line: <spec param flag> value1 value2
    // using %_1% %_2%
    class DriDefineAndMacroList
    {
        struct Pair
        {
            QString Name;
            QString Value;
            bool ReadOnly;      // forbid to modify or undef

            Pair(const QString &name, const QString &val, bool readOnly) 
                : Name(name), Value(val), ReadOnly(readOnly)
            {
            }
        };

        std::vector<Pair> m_list;
        
        bool m_debug;

        static bool NameSyntaxIsOk(const QString &name);
        static void DefineErr(const QString &desc, const QString &src = "",
                              const QString &reason = "");
        static void MacroErr(const QString &desc, const QString &src = "",
                             const QString &reason = "");
        static void Out(const QString &data); // for debug

        void SaveMacroReplaceList(const QString &str);
        void SaveSpecialParamsList(const QStringList &sl);
        
        void ProcessDefineParams(QString params); // can throw
        void ProcessUndefParams(QString name); // can throw

        void AddDefine(const QString &name, const QString &value);
        void AddMacro(const QString &name, const QString &value);
        void AddSpecParam(int name, const QString &value);
        void Add(const QString &name, const QString &value, bool readOnly);

        int Find(const QString &name);
        
        QString ReplaceDefinedNames(QString line); // can throw
        // returns replaced size 
        static int ReplaceWithQuotesConsideration(QString &where, int pos, int size, 
                                                  QString what);
        
    public:
        ESS_TYPEDEF(DriDefineError);
        ESS_TYPEDEF(DriMacroError);

        // Parsing:
        // specialParamsList - params with names like _1, _2, etc.
        // macroReplaceList = "macroName = value; macroName2 = value2",
        // Quotes values that contain system symbols ('.', etc) 
        // (macroparams used in CS_Exec)
        DriDefineAndMacroList(const QStringList &specialParamsList,
                              const QString &macroReplaceList); // can throw
        
        // Returns processed line:
        // - empty, if line contains #define or #undef
        // - with replaced macrosses if has any  
        // - original
        QString ProcessLine(const QString &line); // can throw
    };
    
} // namespace DRI

#endif
