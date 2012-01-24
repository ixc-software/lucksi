#ifndef LISTIE_H
#define LISTIE_H

#include "stdafx.h"

#include "Utils/ErrorsSubsystem.h"
#include "Utils/ManagedList.h"

#include "ICommonIe.h"
#include "isdnpack.h"
#include "isdninfr.h"

namespace ISDN
{    

    class IIsdnL3Internal;
    class IeFactory;


    // Список информационных элементов
    // Разбирает пакет второго уровня формируя список создаваемых через IeFactory инф. элементов. (на приеме)
    // Формирует вектор "сырых данных" из добавляемых Ie (на передаче)
    class ListIe : boost::noncopyable
    {
        typedef std::vector<byte> VectorByte;

        IsdnInfra& m_infra;
        Utils::ManagedList<ICommonIe> m_ies;
        IeFactory& m_ieFactory;
        bool m_isParsed;

    public:

        ESS_TYPEDEF(ParseError);

        ListIe(IsdnInfra& infra, IeFactory& ieFactory);
        
        template<class T>
        void FindIes(std::list<T*> &listIes) const;

        void Parse(QVector<byte> l2data);

        std::string GetAsString(bool withContent = false)const;       

        void GetExistId(std::multiset<IeId>& out) const;

    private:

        void InsertIe(ICommonIe* pIe);

        //void ClearAll();        

        bool NextIe(/* out */ IeId& out, /* out */ int &beginIePos,
            const QVector<byte> &v, int &currPos, int &lockedShift);

        int GetHeaderSize(QVector<byte> l2data);
        
        template<class T>
        void StaticCheckInheritance() const;

    };

    //----------------------------------------------------------------------------------------   

    template<class T>
    void ListIe::FindIes(std::list<T*> &listIes) const
    {
        //add T is base..
        ESS_ASSERT(m_isParsed);
        StaticCheckInheritance<T>();
        IeId id = T::GetId();

        for(int i = 0; i < m_ies.Size(); ++i)
        {
            if (m_ies[i]->GetIeId() == id)
            {
                T *pIe = dynamic_cast<T*>( m_ies[i] );
                ESS_ASSERT(pIe != 0);
                listIes.push_back( pIe );
            }
        }

    }

    template<class T>
    void ListIe::StaticCheckInheritance() const //сборочная проверка CommonIe base of T
    {
        enum { CInherCheck = boost::is_base_of<ICommonIe, T>::value };
        BOOST_STATIC_ASSERT( CInherCheck );
    }

} // ISDN


#endif

