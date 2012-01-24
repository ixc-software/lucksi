#ifndef L3PROFILES_H
#define L3PROFILES_H

#include "Utils/IBasicInterface.h"
#include "Utils/SafeRef.h"
#include "DssTimersProf.h"
#include "IsdnRole.h"
#include "SetBCannels.h"
#include "IeConstants.h"
#include "DssTraceOption.h"
#include "IsdnUtils.h"


namespace ISDN
{
    class IDssToGroup;

    //// Интерфейс профайла для 3уровня
    class L3Profile
    {
        enum         
        {
            CTwoByteCRef = 0x7FFF,
            COneByteCRef = 0x7F
        };

    public:

        struct Options
        {
            bool m_SendConnectAck; //annex D, A.3/Q931(sheet 7) send PacketConnectAck if true
            IeConstants::Location m_Location;
            bool m_SendCallingIfExist; // do send IeCallingPN in setup message
            bool m_SendProgInd; // do send IeProgInd  in setup message                        
            bool m_AssignBChannalsIntfId;   // указывать идентификатор интерфейса B-каналов
            bool m_AutoReActivation;        // если L2 деактивировался и не выполнилась реактивация по таймеру, то L3 инициирует активацию

            Options();
            void Validate() const;
            void SetSide(bool forNetwork);
        };

    
    public:                
        Utils::SafeRef<IDssToGroup> getIntfGroupe();
        const Options& GetOptions()const;
        bool IsUserSide() const;
        DssTimersProf& GetTimers();   
        const DssTimersProf& GetTimers()const;       
        const std::string& Name()const;
        int GetMaxCallrefValue()const;
        const DssTraceOption& getTraceOption() const;
        bool getStatActive() const;        

    public:
        // create default profile for UserSideDss
        static boost::shared_ptr<L3Profile> CreateAsUser(const DssTraceOption& trace, 
            Utils::SafeRef<IDssToGroup> intfGroup,
			const std::string &prefix = "",
            bool statActive = true);

        // create default profile for NetworkSideDss
        static boost::shared_ptr<L3Profile> CreateAsNet(
            const DssTraceOption& trace, 
            Utils::SafeRef<IDssToGroup> intfGroup,
			const std::string &prefix = "",
            bool statActive = true);                
        
        // tune trace options:
        DssTraceOption& getTraceOption();

        // tune Dss-option
        Options& GetOptions();        

        const L3Profile &ValidateFields() const; // assert if error, return this if ok       

    private:        
        L3Profile(){}
        void DefaultInit(const DssTraceOption& trace);

        DssTimersProf m_timers;                       
        std::string m_L3Name; // имя объекта. используется как префикс лог-сессии        
        int m_maxCallRefValue;
        DssTraceOption m_trace;
        bool m_statActive;
        bool m_userSide;
        Utils::SafeRef<IDssToGroup> m_intfGroup;        
        Options m_option;
    };                


} // namespace ISDN

#endif
