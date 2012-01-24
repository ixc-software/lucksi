
#include "stdafx.h"
#include "DriDefineAndMacroList.h"
#include "DRIParserConsts.h"
#include "Utils/QtHelpers.h"
#include "Utils/ExeName.h"
#include "Utils/IntToString.h"
#include "Domain/Identificator.h"
#include "driutils.h"

namespace
{
	const QString CDefine = "#define";
	const QString CUndef = "#undef";
	const QChar CDefNameBreak = '%';

    const QChar CStringChar = '\"';

    const int CMaxSpecParamCount = 100;
    const char CSpecParamChar = '_';

    // ------------------------------------------------------
    
    QString SpecParamName(int num)
    {
        return QString("%1%2").arg(CSpecParamChar).arg(num);
    }

    // ------------------------------------------------------
    
    QString QuoteStr(const QString &str)
    {
        return QString("%1%2%1").arg(CStringChar).arg(str);
    }

    // ------------------------------------------------------

    bool IsDecOrHexNumber(const QString &str)
    {
        Platform::dword num;

        if (Utils::StringToInt(str.toStdString(), num, true)) return true;
        
        return (Utils::HexStringToInt(str.toStdString(), num, true));
    }

    // ------------------------------------------------------

    
} // namespace

// ----------------------------------------------------------------------

namespace DRI
{
    DriDefineAndMacroList::DriDefineAndMacroList(const QStringList &specialParamsList,
                                                 const QString &macroReplaceList) // can throw
	{
        m_debug = false;

        m_list.reserve(CMaxSpecParamCount);

        SaveSpecialParamsList(specialParamsList);
        SaveMacroReplaceList(macroReplaceList);
	}

    // ----------------------------------------------------------------------

    void DriDefineAndMacroList::SaveMacroReplaceList(const QString &str)
    {   
        if (m_debug) Out(QString("Macro list: ").append(str));

        if (str.contains(CStringChar))
        {
            MacroErr(QString("Parameter list contains '%1'").arg(CStringChar), str);
        }
        
        QStringList sl = str.split(";", QString::SkipEmptyParts);

		for(int i = 0; i < sl.size(); ++i)
		{
			QString s = sl.at(i);
			int pos = s.indexOf("=");
			if (pos < 0) MacroErr("Missed '='", str);

			QString name(s.left(pos).trimmed());
            QString val(s.mid(pos + 1).trimmed());
            
            AddMacro(name, val);
		}
    }

    // ----------------------------------------------------------------------

    void DriDefineAndMacroList::SaveSpecialParamsList(const QStringList &sl)
    {
        // _0 - exe name
        AddSpecParam(0, Utils::ExeName::GetExeName().c_str());

        // _1 .. _X - not empty params
        for (int i = 0; i < sl.size(); ++i)
        {
            AddSpecParam(i + 1, sl.at(i));
        }

        // _(X + 1) .. _Max - empty params
        for (int i = sl.size() + 1; i < CMaxSpecParamCount; ++i)
        {
            AddSpecParam(i, "");
        }
    }

    // ----------------------------------------------------------------------
    
    // static
    bool DriDefineAndMacroList::NameSyntaxIsOk(const QString &name)
    {
        return Domain::Identificator::CheckObjectName(name);
    }
    
	// ----------------------------------------------------------------------
    
	// static
	void DriDefineAndMacroList::DefineErr(const QString &desc, const QString &line, 
										  const QString &reason)
	{
		QString msg(desc);
		if (!reason.isEmpty()) msg += QString(": %1").arg(reason);
		if (!line.isEmpty()) msg += QString(", in line '%1'").arg(line);

        ESS_THROW_MSG(DriDefineError, msg.toStdString());
    }

	// ----------------------------------------------------------------------

    // static
	void DriDefineAndMacroList::MacroErr(const QString &desc, const QString &src,
										 const QString &reason)
	{
		QString msg(desc);
		if (!reason.isEmpty()) msg += QString(": %1").arg(reason);
		if (!src.isEmpty()) msg += QString(", in '%1'").arg(src);

		ESS_THROW_MSG(DriMacroError, msg.toStdString());
	}

    // ----------------------------------------------------------------------
    
    // static 
    void DriDefineAndMacroList::Out(const QString &data)
    {
        std::cout << data << std::endl;
    }

	// ----------------------------------------------------------------------
    
	// name value
	// name and value can be in quotes
    // all text after name considered as value
	void DriDefineAndMacroList::ProcessDefineParams(QString params)
	{
        if (m_debug) Out(QString("Define params: ").append(params));

        params = params.trimmed();
		if (params.isEmpty()) DefineErr("#define without params", "", params);

        
        int firstSpace = params.indexOf(" ");

        QString name = params.left(firstSpace);
        QString val = params.mid(name.size() + 1).trimmed(); // can be empty
        if (val.contains("#")) DefineErr("#define value contains '#'", "", params);
        
        if (name.startsWith(CStringChar) && name.endsWith(CStringChar))
        {
            name.remove(0, 1).chop(1);
        }

        if (m_debug) Out(QString("Save define: %1 = %2").arg(name).arg(val));

        AddDefine(name, val); 
	}

	// ----------------------------------------------------------------------

    void DriDefineAndMacroList::AddDefine(const QString &name, const QString &value)
    {
        // check name syntax
        if (!NameSyntaxIsOk(name)) DefineErr("Invalid name syntax", "", name);
        // check for dublicate names
        if (Find(name) >= 0) DefineErr("Dublicate name", "", name);
        
        Add(name, value, false);
    }

    // ----------------------------------------------------------------------

    void DriDefineAndMacroList::AddMacro(const QString &name, const QString &value)
    {
        // check name syntax
        if (!NameSyntaxIsOk(name)) MacroErr("Invalid name syntax", "", name);
        // check for dublicate names
        if (Find(name) >= 0) MacroErr("Dublicate name", "", name);
        
        Add(name, /*QuoteStr(value)*/value, true);
    }

    // ----------------------------------------------------------------------

    void DriDefineAndMacroList::AddSpecParam(int name, const QString &value)
    {
        ESS_ASSERT((name >= 0) && (name < CMaxSpecParamCount));
        
        QString nameStr = SpecParamName(name);

        QString addVal = (NameSyntaxIsOk(value) || IsDecOrHexNumber(value))? 
                          value : QuoteStr(value);
        
        Add(nameStr, addVal, true);
    }

    // ----------------------------------------------------------------------

    void DriDefineAndMacroList::Add(const QString &name, const QString &value, bool readOnly)
    {
        if (m_debug) Out(QString("Add: Name = %1; Value = %2").arg(name).arg(value));
        
        m_list.push_back(Pair(name, value, readOnly));
    }

    // ----------------------------------------------------------------------

	int DriDefineAndMacroList::Find(const QString &name)
	{
		if(m_debug) Out(QString("Search for %1 in list[%2]").arg(name).arg(m_list.size()));
        
		for (int i = 0; i < m_list.size(); ++i)
		{
            if (m_list.at(i).Name == name) return i;
		}

		return -1;
	}

	// ----------------------------------------------------------------------

    void DriDefineAndMacroList::ProcessUndefParams(QString name)
    {
        // params = name
        name = name.trimmed();

        if (m_debug) Out(QString("Trying to remove define: ").append(name));
        
        if (name.isEmpty()) DefineErr("#undef without name", "", name);
        
        int index = Find(name);
        if (index < 0) DefineErr("Unknown name in #undef", "", name);

        if (m_list.at(index).ReadOnly) 
        {
            MacroErr("Trying to undefine macross or special parameter", "", name);
        }

        m_list.erase(m_list.begin() + index);
    }

    // ----------------------------------------------------------------------

	// Returns processed line:
	// - empty, if line contains #define or #undef
	// - with replaced macrosses if has any  
	// - original
	QString DriDefineAndMacroList::ProcessLine(const QString &line)
	{
        if (m_debug) Out(QString("Process line '%1'").arg(line));
        
		QString res = ReplaceDefinedNames(line);

        if (res.startsWith(CDefine)) 
        {
            ProcessDefineParams(res.mid(CDefine.size()));
            res.clear();
        }
        
		if (res.startsWith(CUndef))
        { 
            ProcessUndefParams(res.mid(CUndef.size()));
            res.clear();
        }

        return res;
	}
   
	// ----------------------------------------------------------------------

    int DriDefineAndMacroList::ReplaceWithQuotesConsideration(QString &where, 
                                                              int pos, 
                                                              int size, 
                                                              QString what)
    {
        ESS_ASSERT(pos < where.size());
        
        int findFrom = 0;
        bool insideBreaks = false;
        while (true)
        {
            findFrom = where.indexOf(CStringChar, findFrom);
            if ((findFrom < 0) || (findFrom > pos)) break;
            
            insideBreaks = !insideBreaks;
            ++findFrom;
        }

        if (insideBreaks) what = DriUtils::UnquoteString(what, CStringChar);

        // if (what.isEmpty()) what = QString(2, CStringChar);

        where.replace(pos, size, what);

        return what.size(); 
    }
    
    // ----------------------------------------------------------------------

    QString DriDefineAndMacroList::ReplaceDefinedNames(QString line)
	{
        if (m_debug) Out("Replace defined names");
        
        int i = 0;
		while (i < line.size())
		{
		    if (line.at(i) != CDefNameBreak)
			{
				++i;
				continue;
			}

			int rightBreak = line.indexOf(CDefNameBreak, i + 1);
			if (rightBreak < 0) break;

			QString name = line.mid(i + 1, rightBreak - i - 1);

            if (!NameSyntaxIsOk(name))
            {
                i = rightBreak;
                continue; 
            }
            
			int nameInx = Find(name);
			if (nameInx < 0) DefineErr("Unknown defined name", line, name);

            int toReplSize = name.size() + 2; // %name%
			int replacedSize = ReplaceWithQuotesConsideration(line, i, toReplSize, 
                                                              m_list.at(nameInx).Value);
            
			i += replacedSize;
        }

        if (m_debug) Out("\\Replace defined names");
        
		return line;
    }
    
} // namespace DRI

