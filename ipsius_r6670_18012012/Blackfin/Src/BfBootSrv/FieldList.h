#ifndef FIELDLIST_H
#define FIELDLIST_H

#include "IRegField.h"
#include "CfgField.h"
#include "ConfigLineList.h"



namespace BfBootSrv
{
    
    // Список полей конфигурационной структуры Config
    class FieldList : public IRegField
    {
        typedef std::vector<ICfgField*> CollectField;
        CollectField m_list;

        void RegField(ICfgField& newField)
        {
            // todo ASSERT duplicate
            m_list.push_back(&newField);
        }
    public:

        //в ConfigLineList обязанны быть все поля
        void InitFieldsFrom(const ConfigLineList& list) // can throw ParametrNotFound
        {        
            try
            {
                for (int iField = 0; iField < m_list.size(); ++iField) 
                {
                    BfBootCore::ConfigLine line = list.FindLineByName( m_list[iField]->getName() );
                    m_list[iField]->setValue(line);
                }            
            }
            catch(BfBootCore::StringToTypeConverter::CantConvert&)
            {
                ESS_THROW(ErrCantConvertVal);
            }
            
        }

        void WriteTo(ConfigLineList& list/*, bool writeHide = false*/)
        {
            for (int iField = 0; iField < m_list.size(); ++iField) 
            {
                list.AddLine( m_list[iField]->ToCfgLine() );
            }
        }

        // return false if invalid pair name-key
        bool SetParam(const std::string& name, CfgFieldProp::Key key, const std::string& val )
        {
            for (int iField = 0; iField < m_list.size(); ++iField) 
            {
                ICfgField& field = *m_list[iField];
                
                if (field.getName() == name &&  field.getFindMark() == key)
                {
                    return field.setValue(BfBootCore::ConfigLine(name, val));                    
                }                
            }

            return false;
        }    

        void ListSettings(std::vector<BfBootCore::ConfigLine>& out, CfgFieldProp::Key key) const
        {
            for (int iField = 0; iField < m_list.size(); ++iField) 
            {
                if (m_list[iField]->getFindMark() == key) out.push_back(m_list[iField]->ToCfgLine());                
            }
        }

        // for tests
        bool Compare(const FieldList& other) const
        {
            ESS_ASSERT( other.m_list.size() == m_list.size() );

            for (int iField = 0; iField < m_list.size(); ++iField) 
            {
                if ( !m_list[iField]->CompareVal( other.m_list[iField]) ) return false;
            }
            return true;
        }

        

    };
} // namespace BfBootSrv

#endif
