#ifndef L3PACKET_H
#define L3PACKET_H

#include "stdafx.h"
#include "Platform/PlatformTypes.h"
#include "Utils/ErrorsSubsystem.h"
//#include "Utils/ToBool.h"
#include "Utils/BaseSpecAllocated.h"

#include "ListIe.h"
#include "isdninfr.h"
#include "IIsdnLog.h"
#include "DssWarning.h"
#include "IIsdnL3Internal.h"
#include "L3MessageType.h"

namespace ISDN
{   
	class CallRef;
    class IeFactory;
    class IsdnL3;

    // ---------------------------------------------------------------

    class CallFsmBaseEvent
    {
    public:
        virtual ~CallFsmBaseEvent() {}
    };

    // ---------------------------------------------------------------

    // Обобщенный тип пакета 3го уровня
    class L3Packet : 
        public Utils::BaseSpecAllocated<IIsdnAlloc>, 
        public CallFsmBaseEvent,
        boost::noncopyable
    {
        typedef std::vector<byte> VectorByte;       
        enum {CProtDescriptor = 0x8};

    public:

        // Constructor of incoming call (use from L3PacketFactory)
        L3Packet (QVector<byte> l2data, IsdnInfra& infra, IeFactory& ieFactory);

        // Constructor of outgoing call (use from L3Call)
		L3Packet (const CallRef &callref, IIsdnL3Internal& IL3, L3MessgeType::Type type);

		static L3MessgeType::Type ExtractMessageType(IIsdnAlloc& alloc, const QVector<byte> &data);

        // implemented in L3PacketTypes
        virtual void UpRout(L3Call &call) = 0;
		L3MessgeType::Type Type() const {	return m_type; }

		template<class T>
        int FindIe(T* &pIe) const;         
        
        template<class T>
        int FindIes(std::list<T*> &listIes) const;

        void AddIe(const ICommonIe& Ie);       

        VectorByte GetRaw() const;

        const std::string &GetAsString(bool withIeContent) const;
        const std::string& GetName()const;

        bool ValidateMandatoryIes();

		CallRef GetCallref() const 
		{
			return m_callref;
		}

        bool UpRoutAsGlobal(IsdnL3& receiver)
        {
            if (!IsGlobalRouting()) return false;
            if (!m_callref.IsGlobal()) return false;
            UpRout(receiver);
            return true;
        }
    private:        
        
        static bool IsBroadcast(QVector<byte> l2data)
        {
            //return (p->GetByIndex(1) == 0); 
            return l2data.at(1) == 0;
        }        

        void RememberAdded(IeId id)
        {
            ESS_ASSERT(!m_incomming);
            m_addedIes.insert(id);
        }

        template<class T>
        void StaticCheckInheritance() const;
            
        virtual bool IeCorrectForThisMsgType(IeId) = 0; // проверка добавляемого IE     
        virtual bool ValidateMandatoryIes(const std::multiset<IeId>& exist) const = 0;

        // Перекрываются только в пакетах относящимся ко всем вызовам 
        // (GlobalCrefState processing)
        virtual void UpRout(IsdnL3& receiver)
        {
            ESS_HALT("Is not global cref packet");
        }
        virtual bool IsGlobalRouting() const // участвует ли пакет данногно типа в глбальной маршрутизации
        {
            return false;
        }
        

	private:		
		L3MessgeType::Type m_type;
        IsdnInfra& m_infra;
        mutable ListIe m_ListIe;
        VectorByte m_IeData;
        bool m_incomming;
		CallRef m_callref;
        std::multiset<IeId> m_addedIes;
        mutable std::string m_myName;
        mutable std::string m_myContent;
        
        /*полей pd, mt нет т.к они есть свойство типа*/

    };

    //-------------------------------------------------------------------------------------

    /* Общая часть типизированного пакета */    
    template<L3MessgeType::Type MsgType> 
    class PacketType : public L3Packet
    {
        
    public:

        static L3MessgeType::Type GetMsgType() { return MsgType; }        
                      
        PacketType(QVector<byte> l2data, IsdnInfra& infra, IeFactory& ieFactory) 
            : L3Packet(l2data, infra, ieFactory) {}

        PacketType(const CallRef &callref, IIsdnL3Internal& IL3) 
            : L3Packet(callref, IL3, MsgType) {}
   
    };

    //-------------------------------------------------------------------------------------

    // После уничтожения пакета указатель на Ie становится недействительным!
    template<class T>
    int L3Packet::FindIe(T* &pIe) const
    {        
        StaticCheckInheritance<T>();
        ESS_ASSERT(m_incomming);

        std::list<T*> listIes;
        if ( FindIes(listIes) ) 
        {
            pIe = listIes.front();            
        }
        return listIes.size();
    }


    // После уничтожения пакета указатели на Ie становятся недействительными!
    template<class T>
    int L3Packet::FindIes(std::list<T*> &listIes) const
    {
        ESS_ASSERT(m_incomming);
        StaticCheckInheritance<T>();     
        ESS_ASSERT ( !listIes.size() ); // врядли есть смысл дописывать в уже заполненый список

        m_ListIe.FindIes(listIes);        

        return listIes.size();            
    }
   

    template<class T>
    void L3Packet::StaticCheckInheritance() const //сборочная проверка CommonIe base of T
    {
        enum { CInherCheck = boost::is_base_of<ICommonIe, T>::value };
        BOOST_STATIC_ASSERT( CInherCheck );
    }

     
} // namespace ISDN

#endif

